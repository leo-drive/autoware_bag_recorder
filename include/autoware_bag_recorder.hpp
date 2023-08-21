// Copyright 2023 TIER IV, Inc., Leo Drive Teknoloji A.Ş.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef AUTOWARE_BAG_RECORDER_HPP_
#define AUTOWARE_BAG_RECORDER_HPP_

#include "rclcpp/rclcpp.hpp"
#include "rosbag2_cpp/converter_options.hpp"
#include "rosbag2_cpp/reader.hpp"
#include "rosbag2_cpp/typesupport_helpers.hpp"
#include "rosbag2_cpp/writer.hpp"

#include <rosbag2_storage/rosbag2_storage/storage_options.hpp>

#include <tier4_control_msgs/msg/gate_mode.hpp>

#include <sys/statfs.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace autoware_bag_recorder
{

struct TopicInfo
{
  std::string topic_name;
  std::string topic_type;
};

struct ModuleSection
{
  std::string folder_path;
  std::vector<TopicInfo> topic_info;
  std::unique_ptr<rosbag2_cpp::Writer> bag_writer;
  std::vector<std::string> topic_names;
};

class AutowareBagRecorderNode : public rclcpp::Node
{
public:
  AutowareBagRecorderNode(const std::string & node_name, const rclcpp::NodeOptions & options);

private:
  void initialize_parameters();
  void setup_module_sections();
  void setup_single_module(
    const std::string & module_param, std::vector<std::string> & topics,
    const std::string & section_name);
  void record_all_topics_in_a_bag();
  void check_and_remove_files_at_init();
  void initialize_bag_files_for_topics();
  void start_topic_search();
  void start_status_control();
  void check_disk_space();
  void check_record_time(
    const std::chrono::time_point<std::chrono::system_clock> & start_record_time);
  void check_bag_time(std::chrono::time_point<std::chrono::system_clock> & start_bag_time);
  void check_auto_mode();
  static std::string get_timestamp();
  rclcpp::QoS get_qos_profile_of_topic(const std::string & topic_name);
  void rotate_topic_names(autoware_bag_recorder::ModuleSection & section);
  void search_topic(ModuleSection & section);
  static void create_bag_file(
    std::unique_ptr<rosbag2_cpp::Writer> & writer, const std::string & bag_path);
  void bag_file_handler();
  static void add_topics_to_writer(
    std::unique_ptr<rosbag2_cpp::Writer> & writer_, std::string topic_name, std::string topic_type);
  void generic_subscription_callback(
    const std::shared_ptr<rclcpp::SerializedMessage const> & msg, const std::string & topic_name,
    autoware_bag_recorder::ModuleSection & section);
  void section_factory(const std::vector<std::string> & topics, const std::string & path);
  static double get_root_disk_space();
  void remove_remainder_bags_in_folder(autoware_bag_recorder::ModuleSection & section) const;
  void free_disk_space_for_continue(autoware_bag_recorder::ModuleSection & section) const;
  static void check_files_in_folder(
    autoware_bag_recorder::ModuleSection & section, std::vector<std::string> & directories);
  void gate_mode_cmd_callback(const tier4_control_msgs::msg::GateMode::ConstSharedPtr msg);
  void run();

  // parameters
  int maximum_record_time_;
  int bag_time_;
  std::string bag_path_;
  int disk_space_threshold_;
  int number_of_maximum_bags_;
  std::string disk_space_action_mode_;

  bool record_all_topic_in_a_bag_;
  bool enable_only_auto_mode_recording_;
  bool is_writing_;
  bool record_api_topics_;
  bool record_autoware_topics_;
  bool record_control_topics_;
  bool record_external_topics_;
  bool record_localization_topics_;
  bool record_map_topics_;
  bool record_perception_topics_;
  bool record_planning_topics_;
  bool record_sensing_topics_;
  bool record_system_topics_;
  bool record_vehicle_topics_;

  int remaining_topic_num_;

  std::vector<std::string> api_topics_;
  std::vector<std::string> autoware_topics_;
  std::vector<std::string> control_topics_;
  std::vector<std::string> external_topics_;
  std::vector<std::string> localization_topics_;
  std::vector<std::string> map_topics_;
  std::vector<std::string> perception_topics_;
  std::vector<std::string> planning_topics_;
  std::vector<std::string> sensing_topics_;
  std::vector<std::string> system_topics_;
  std::vector<std::string> vehicle_topics_;

  std::vector<ModuleSection> module_sections_;

  std::vector<rclcpp::SubscriptionBase::SharedPtr> subscriptions_;
  rclcpp::Subscription<tier4_control_msgs::msg::GateMode>::ConstSharedPtr gate_mode_sub_;
  rclcpp::Node::SharedPtr node_;

  std::shared_ptr<rclcpp::SerializedMessage> serialized_msg_ptr_;
  tier4_control_msgs::msg::GateMode::ConstSharedPtr gate_mode_msg_ptr;

  std::mutex writer_mutex_;
};

}  // namespace autoware_bag_recorder
#endif  // AUTOWARE_BAG_RECORDER_HPP_