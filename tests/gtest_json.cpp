#include <gtest/gtest.h>
#include "behaviortree_cpp/blackboard.h"
#include "behaviortree_cpp/json_export.h"
#include "behaviortree_cpp/basic_types.h"

namespace
{
constexpr std::string_view kTypeField = "__type";
constexpr std::string_view kValueField = "value";
}  // namespace

//----------- Custom types ----------

namespace TestTypes
{

struct Vector3D
{
  double x = 0;
  double y = 0;
  double z = 0;
};

struct Quaternion3D
{
  double w = 1;
  double x = 0;
  double y = 0;
  double z = 0;
};

struct Pose3D
{
  Vector3D pos;
  Quaternion3D rot;
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
  exporter.toJson(BT::Any("string_val"), json["string"]);
  exporter.toJson(BT::Any(69), json["int"]);
  exporter.toJson(BT::Any(static_cast<uint64_t>(96)), json["uint"]);
  exporter.toJson(BT::Any(3.14), json["real"]);
  exporter.toJson(BT::Any(pose), json["pose"]);

  std::cout << json.dump(2) << std::endl;

  ASSERT_EQ(json["string"][kTypeField], "string");
  ASSERT_EQ(json["string"][kValueField], "string_val");

  ASSERT_EQ(json["int"][kTypeField], "int64_t");
  ASSERT_EQ(json["int"][kValueField], 69);

  ASSERT_EQ(json["uint"][kTypeField], "uint64_t");
  ASSERT_EQ(json["uint"][kValueField], 96);

  ASSERT_EQ(json["real"][kTypeField], "double");
  ASSERT_EQ(json["real"][kValueField], 3.14);

  ASSERT_EQ(json["pose"][kTypeField], "Pose3D");
  ASSERT_EQ(json["pose"]["pos"]["x"], 1);
  ASSERT_EQ(json["pose"]["pos"]["y"], 2);
  ASSERT_EQ(json["pose"]["pos"]["z"], 3);

  ASSERT_EQ(json["pose"]["rot"]["w"], 4);
  ASSERT_EQ(json["pose"]["rot"]["x"], 5);
  ASSERT_EQ(json["pose"]["rot"]["y"], 6);
  ASSERT_EQ(json["pose"]["rot"]["z"], 7);

  // check the two-ways transform, i.e. "from_json"
  auto pose2 = exporter.fromJson(json["pose"])->first.cast<TestTypes::Pose3D>();

  ASSERT_EQ(pose.pos.x, pose2.pos.x);
  ASSERT_EQ(pose.pos.y, pose2.pos.y);
  ASSERT_EQ(pose.pos.z, pose2.pos.z);

  ASSERT_EQ(pose.rot.w, pose2.rot.w);
  ASSERT_EQ(pose.rot.x, pose2.rot.x);
  ASSERT_EQ(pose.rot.y, pose2.rot.y);
  ASSERT_EQ(pose.rot.z, pose2.rot.z);

  auto string_val = exporter.fromJson(json["string"])->first.cast<std::string>();
  ASSERT_EQ(string_val, "string_val");
  auto int_val = exporter.fromJson(json["int"])->first.cast<int>();
  ASSERT_EQ(int_val, 69);
  auto uint_val = exporter.fromJson(json["uint"])->first.cast<uint>();
  ASSERT_EQ(uint_val, 96);
  auto real = exporter.fromJson(json["real"])->first.cast<double>();
  ASSERT_EQ(real, 3.14);
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
