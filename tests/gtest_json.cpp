#include <gtest/gtest.h>
#include "behaviortree_cpp/blackboard.h"
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
namespace nlohmann
{
void to_json(json& j, const Vector3D& v)
{
  // compact syntax
  j = {{"x", v.x}, {"y", v.y}, {"z", v.z}};
}

void to_json(json& j, const Quaternion3D& q)
{
  // verbose syntax
  j["w"] = q.w;
  j["x"] = q.x;
  j["y"] = q.y;
  j["z"] = q.z;
}

void to_json(json& j, const Pose3D& p)
{
  j = {{"pos", p.pos}, {"rot", p.rot}};
}
} // end namespace nlohmann


using namespace BT;

TEST(JsonTest, Basic)
{
  JsonExporter exporter;

  nlohmann::json json;
  exporter.toJson(69, json["plain_int"]);
  exporter.toJson(BT::Any(71), json["any_int"]);

  exporter.toJson(M_PI, json["plain_real"]);
  exporter.toJson(BT::Any(M_PI_2), json["any_real"]);

  nlohmann::json json_expected;
  json_expected["any_int"] = 71;
  json_expected["any_real"] = M_PI_2;
  json_expected["plain_int"] = 69;
  json_expected["plain_real"] = M_PI;

  ASSERT_EQ(json_expected, json);
}

TEST(JsonTest, AnyConversion)
{
  JsonExporter exporter;

  Pose3D pose = { {1,2,3},
                  {4,5,6,7} };

  nlohmann::json json;
  exporter.toJson(BT::Any(69), json["int"]);
  exporter.toJson(BT::Any(M_PI), json["real"]);

  // expected to throw, because we haven't called addConverter()
  ASSERT_THROW( exporter.toJson(BT::Any(pose), json["pose"]), std::logic_error );

  // now it should work
  exporter.addConverter<Pose3D>();
  exporter.toJson(BT::Any(pose), json["pose"]);

  nlohmann::json json_expected;
  json_expected["int"] = 69;
  json_expected["real"] = M_PI;

  json_expected["pose"]["pos"]["x"] = 1;
  json_expected["pose"]["pos"]["y"] = 2;
  json_expected["pose"]["pos"]["z"] = 3;

  json_expected["pose"]["rot"]["w"] = 4;
  json_expected["pose"]["rot"]["x"] = 5;
  json_expected["pose"]["rot"]["y"] = 6;
  json_expected["pose"]["rot"]["z"] = 7;

  ASSERT_EQ(json_expected, json);

}




