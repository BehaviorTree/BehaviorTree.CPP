#include <gtest/gtest.h>
#include "behaviortree_cpp/blackboard.h"
#include "behaviortree_cpp/json_export.h"
#include "behaviortree_cpp/basic_types.h"

//----------- Custom types ----------

namespace TestTypes
{

struct Vector3D
{
  double x = 0;
  double y = 0;
  double z = 0;
  bool operator==(const Vector3D& other) const
  {
    return x == other.x && y == other.y && z == other.z;
  }
};

struct Quaternion3D
{
  double w = 1;
  double x = 0;
  double y = 0;
  double z = 0;
  bool operator==(const Quaternion3D& other) const
  {
    return w == other.w && x == other.x && y == other.y && z == other.z;
  }
};

struct Pose3D
{
  Vector3D pos;
  Quaternion3D rot;
  bool operator==(const Pose3D& other) const
  {
    return pos == other.pos && rot == other.rot;
  }
};

struct Time
{
  uint32_t sec;
  uint32_t nsec;
};

BT_JSON_CONVERTER(Vector3D, v)
{
  add_field("x", &v.x);
  add_field("y", &v.y);
  add_field("z", &v.z);
}

BT_JSON_CONVERTER(Quaternion3D, v)
{
  add_field("w", &v.w);
  add_field("x", &v.x);
  add_field("y", &v.y);
  add_field("z", &v.z);
}

BT_JSON_CONVERTER(Pose3D, v)
{
  add_field("pos", &v.pos);
  add_field("rot", &v.rot);
}

// specialized functions
void jsonFromTime(const Time& t, nlohmann::json& j)
{
  j["stamp"] = double(t.sec) + 1e-9 * double(t.nsec);
}

void jsonToTime(const nlohmann::json& j, Time& t)
{
  double sec = j["stamp"];
  t.sec = int(sec);
  t.nsec = (sec - t.sec) * 1e9;
}

}  // namespace TestTypes

//----------- JSON specialization ----------

class JsonTest : public testing::Test
{
protected:
  JsonTest()
  {
    BT::JsonExporter& exporter = BT::JsonExporter::get();
    exporter.addConverter<TestTypes::Pose3D>();
    exporter.addConverter<TestTypes::Vector3D>();
    exporter.addConverter<TestTypes::Quaternion3D>();

    exporter.addConverter<TestTypes::Time>(TestTypes::jsonFromTime);
    exporter.addConverter<TestTypes::Time>(TestTypes::jsonToTime);
  }
};

TEST_F(JsonTest, TwoWaysConversion)
{
  BT::JsonExporter& exporter = BT::JsonExporter::get();

  TestTypes::Pose3D pose = { { 1, 2, 3 }, { 4, 5, 6, 7 } };

  nlohmann::json json;
  exporter.toJson(BT::Any(69), json["int"]);
  exporter.toJson(BT::Any(3.14), json["real"]);
  exporter.toJson(BT::Any(pose), json["pose"]);

  std::cout << json.dump(2) << std::endl;

  ASSERT_EQ(json["int"], 69);
  ASSERT_EQ(json["real"], 3.14);

  ASSERT_EQ(json["pose"]["__type"], "Pose3D");
  ASSERT_EQ(json["pose"]["pos"]["x"], 1);
  ASSERT_EQ(json["pose"]["pos"]["y"], 2);
  ASSERT_EQ(json["pose"]["pos"]["z"], 3);

  ASSERT_EQ(json["pose"]["rot"]["w"], 4);
  ASSERT_EQ(json["pose"]["rot"]["x"], 5);
  ASSERT_EQ(json["pose"]["rot"]["y"], 6);
  ASSERT_EQ(json["pose"]["rot"]["z"], 7);

  auto num_result = exporter.fromJson(json["int"]);
  ASSERT_TRUE(num_result) << num_result.error();
  auto num = num_result->first.cast<int>();
  ASSERT_EQ(num, 69);

  auto real_result = exporter.fromJson(json["real"]);
  ASSERT_TRUE(real_result) << real_result.error();
  auto real = real_result->first.cast<double>();
  ASSERT_EQ(real, 3.14);

  auto pose_result = exporter.fromJson(json["pose"]);
  ASSERT_TRUE(pose_result) << pose_result.error();
  auto pose_out = pose_result->first.cast<TestTypes::Pose3D>();
  ASSERT_EQ(pose.pos, pose_out.pos);
  ASSERT_EQ(pose.rot, pose_out.rot);
}

TEST_F(JsonTest, CustomTime)
{
  BT::JsonExporter& exporter = BT::JsonExporter::get();

  TestTypes::Time stamp = { 3, 8000000 };
  nlohmann::json json;
  exporter.toJson(BT::Any(stamp), json);
  std::cout << json.dump() << std::endl;

  {
    auto res = exporter.fromJson(json, typeid(TestTypes::Time));
    ASSERT_TRUE(res);
    auto stamp_out = res->first.cast<TestTypes::Time>();
    ASSERT_EQ(stamp.sec, stamp_out.sec);
    ASSERT_EQ(stamp.nsec, stamp_out.nsec);
  }
  {
    auto res = exporter.fromJson(json);
    ASSERT_TRUE(res);
    auto stamp_out = res->first.cast<TestTypes::Time>();
    ASSERT_EQ(stamp.sec, stamp_out.sec);
    ASSERT_EQ(stamp.nsec, stamp_out.nsec);
  }
  {
    auto stamp_out = exporter.fromJson<TestTypes::Time>(json);
    ASSERT_TRUE(stamp_out);
    ASSERT_EQ(stamp.sec, stamp_out->sec);
    ASSERT_EQ(stamp.nsec, stamp_out->nsec);
  }
}

TEST_F(JsonTest, ConvertFromString)
{
  TestTypes::Vector3D vect;
  auto const test_json = R"(json:{"x":2.1, "y":4.2, "z":6.3})";
  ASSERT_NO_THROW(vect = BT::convertFromString<TestTypes::Vector3D>(test_json));

  ASSERT_EQ(vect.x, 2.1);
  ASSERT_EQ(vect.y, 4.2);
  ASSERT_EQ(vect.z, 6.3);
}

TEST_F(JsonTest, BlackboardInOut)
{
  auto bb_in = BT::Blackboard::create();
  bb_in->set("int", 42);
  bb_in->set("real", 3.14);
  bb_in->set("vect", TestTypes::Vector3D{ 1.1, 2.2, 3.3 });

  auto json = ExportBlackboardToJSON(*bb_in);
  std::cout << json.dump(2) << std::endl;

  auto bb_out = BT::Blackboard::create();
  ImportBlackboardFromJSON(json, *bb_out);

  ASSERT_EQ(bb_out->get<int>("int"), 42);
  ASSERT_EQ(bb_out->get<double>("real"), 3.14);

  auto vect_out = bb_out->get<TestTypes::Vector3D>("vect");
  ASSERT_EQ(vect_out.x, 1.1);
  ASSERT_EQ(vect_out.y, 2.2);
  ASSERT_EQ(vect_out.z, 3.3);
}

TEST_F(JsonTest, VectorInteger)
{
  BT::JsonExporter& exporter = BT::JsonExporter::get();

  std::vector<int> vec = { 1, 2, 3 };
  nlohmann::json json;
  exporter.toJson(BT::Any(vec), json["vec"]);

  std::cout << json.dump(2) << std::endl;

  ASSERT_EQ(json["vec"][0], 1);
  ASSERT_EQ(json["vec"][1], 2);
  ASSERT_EQ(json["vec"][2], 3);

  auto result = exporter.fromJson(json["vec"]);
  ASSERT_TRUE(result) << result.error();
  auto vec_out = result->first.cast<std::vector<int>>();

  ASSERT_EQ(vec.size(), vec_out.size());
  for(size_t i = 0; i < vec.size(); ++i)
  {
    ASSERT_EQ(vec[i], vec_out[i]);
  }
}

TEST_F(JsonTest, VectorSring)
{
  BT::JsonExporter& exporter = BT::JsonExporter::get();
  std::vector<std::string> vec = { "hello", "world" };
  nlohmann::json json;
  exporter.toJson(BT::Any(vec), json["vec"]);
  std::cout << json.dump(2) << std::endl;
  ASSERT_EQ(json["vec"][0], "hello");
  ASSERT_EQ(json["vec"][1], "world");
  auto result = exporter.fromJson(json["vec"]);
  ASSERT_TRUE(result) << result.error();
  auto vec_out = result->first.cast<std::vector<std::string>>();
  ASSERT_EQ(vec.size(), vec_out.size());
  for(size_t i = 0; i < vec.size(); ++i)
  {
    ASSERT_EQ(vec[i], vec_out[i]);
  }
  // check the two-ways transform, i.e. "from_json"
  auto result2 = exporter.fromJson(json["vec"]);
  ASSERT_TRUE(result2) << result2.error();
  auto vec_out2 = result2->first.cast<std::vector<std::string>>();
  ASSERT_EQ(vec.size(), vec_out2.size());
  for(size_t i = 0; i < vec.size(); ++i)
  {
    ASSERT_EQ(vec[i], vec_out2[i]);
  }
}

TEST_F(JsonTest, VectorOfCustomTypes)
{
  BT::JsonExporter& exporter = BT::JsonExporter::get();

  std::vector<TestTypes::Pose3D> poses(2);
  poses[0].pos = { 1, 2, 3 };
  poses[0].rot = { 4, 5, 6, 7 };

  poses[1].pos = { 8, 9, 10 };
  poses[1].rot = { 11, 12, 13, 14 };

  nlohmann::json json;
  exporter.toJson(BT::Any(poses), json["poses"]);

  std::cout << json.dump(2) << std::endl;

  ASSERT_EQ(json["poses"][0]["__type"], "Pose3D");
  ASSERT_EQ(json["poses"][0]["pos"]["x"], 1);
  ASSERT_EQ(json["poses"][0]["pos"]["y"], 2);
  ASSERT_EQ(json["poses"][0]["pos"]["z"], 3);
  ASSERT_EQ(json["poses"][0]["rot"]["w"], 4);
  ASSERT_EQ(json["poses"][0]["rot"]["x"], 5);
  ASSERT_EQ(json["poses"][0]["rot"]["y"], 6);
  ASSERT_EQ(json["poses"][0]["rot"]["z"], 7);
  ASSERT_EQ(json["poses"][1]["__type"], "Pose3D");
  ASSERT_EQ(json["poses"][1]["pos"]["x"], 8);
  ASSERT_EQ(json["poses"][1]["pos"]["y"], 9);
  ASSERT_EQ(json["poses"][1]["pos"]["z"], 10);
  ASSERT_EQ(json["poses"][1]["rot"]["w"], 11);
  ASSERT_EQ(json["poses"][1]["rot"]["x"], 12);
  ASSERT_EQ(json["poses"][1]["rot"]["y"], 13);
  ASSERT_EQ(json["poses"][1]["rot"]["z"], 14);

  // check the two-ways transform, i.e. "from_json"
  auto result = exporter.fromJson(json["poses"]);
  ASSERT_TRUE(result) << result.error();
  auto poses_out = result->first.cast<std::vector<TestTypes::Pose3D>>();

  ASSERT_EQ(poses.size(), poses_out.size());

  ASSERT_EQ(poses[0].pos, poses_out[0].pos);
  ASSERT_EQ(poses[0].rot, poses_out[0].rot);

  ASSERT_EQ(poses[1].pos, poses_out[1].pos);
  ASSERT_EQ(poses[1].rot, poses_out[1].rot);
}
