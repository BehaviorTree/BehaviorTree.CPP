#include <gtest/gtest.h>
#include "behaviortree_cpp/json_export.h"

//----------- Custom types ----------

struct Vector3D {
  double x;
  double y;
  double z;
};

struct Quaternion3D {
  double w;
  double x;
  double y;
  double z;
};

struct Pose3D {
  Vector3D pos;
  Quaternion3D rot;
};

//----------- JSON specialization ----------

void to_json(nlohmann::json& j, const Vector3D& v)
{
  // compact syntax
  j = {{"x", v.x}, {"y", v.y}, {"z", v.z}};
}

void to_json(nlohmann::json& j, const Quaternion3D& q)
{
  // verbose syntax
  j["w"] = q.w;
  j["x"] = q.x;
  j["y"] = q.y;
  j["z"] = q.z;
}

void to_json(nlohmann::json& j, const Pose3D& p)
{
  j = {{"pos", p.pos}, {"rot", p.rot}};
}


using namespace BT;

TEST(JsonTest, Exporter)
{
  JsonExporter exporter;

  Pose3D pose = { {1,2,3},
                               {4,5,6,7} };

  nlohmann::json json;
  exporter.toJson(BT::Any(69), json["int"]);
  exporter.toJson(BT::Any(3.14), json["real"]);

  // expected to throw, because we haven't called addConverter()
  ASSERT_FALSE( exporter.toJson(BT::Any(pose), json["pose"]) );

  // now it should work
  exporter.addConverter<Pose3D>();
  exporter.toJson(BT::Any(pose), json["pose"]);

  nlohmann::json json_expected;
  json_expected["int"] = 69;
  json_expected["real"] = 3.14;

  json_expected["pose"]["pos"]["x"] = 1;
  json_expected["pose"]["pos"]["y"] = 2;
  json_expected["pose"]["pos"]["z"] = 3;

  json_expected["pose"]["rot"]["w"] = 4;
  json_expected["pose"]["rot"]["x"] = 5;
  json_expected["pose"]["rot"]["y"] = 6;
  json_expected["pose"]["rot"]["z"] = 7;

  ASSERT_EQ(json_expected, json);

  std::cout << json.dump(2) << std::endl;
}




