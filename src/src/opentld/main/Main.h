/*  Copyright 2011 AIT Austrian Institute of Technology
*
*   This file is part of OpenTLD.
*
*   OpenTLD is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   OpenTLD is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with OpenTLD.  If not, see <http://www.gnu.org/licenses/>.
*
*/

/*
 * main.h
 *
 *  Created on: Nov 18, 2011
 *      Author: Georg Nebehay
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "handler3D.hpp"
#include "Trajectory.h"
#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Time.h"
#include "std_msgs/Duration.h"
#include <geometry_msgs/PolygonStamped.h>
#include <sensor_msgs/Image.h>
#include "TLD.h"
#include "ImAcq.h"
#include "Gui.h"


#include "sensor_msgs/PointCloud2.h"
#include "pcl_ros/point_cloud.h"
// PCL specific includes
#include <pcl/ros/conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include "cv_bridge/cv_bridge.h"
#include <sensor_msgs/image_encodings.h>

#include "open_tld_3d/model.h"
#include <std_msgs/Int32MultiArray.h>


using namespace tld;
using namespace cv;

enum Retval
{
    PROGRAM_EXIT = 0,
    SUCCESS = 1
};

class Main
{
public:
	tld::TLD *tld;
//	ImAcq *imAcq;
	tld::Gui *gui;
	bool showOutput;
	bool showTrajectory;
	int trajectoryLength;
	const char *printResults;
	const char *saveDir;
	double threshold;
	bool showForeground;
	bool showNotConfident;
	bool selectManually;
	int *initialBB;
	bool reinit;
	bool exportModelAfterRun;
	bool loadModel;
	std::string modelPath;
	std::string modelExportFile;
	int seed;
	//New
	bool keyboardControl;
	bool flag;
	bool reuseFrameOnce;
	bool skipProcessingOnce;
	FILE *resultsFile;
	Trajectory trajectory;
	double time_constant;
	int *selection;
  Mat grey;

	//3D
	bool enable3DTracking;
	Handler3D *handy;

	//ROS
	ros::Publisher *poete;
	ros::NodeHandle pnode;
	ros::Subscriber obj_select_sub_;
    ros::Subscriber _parameter_updated_sub;
	//image_transport::Publisher image_pub_;

    struct param{
        std::string global_namespace;
        bool clear_model;
        bool learning_disabled;
        bool detector_disabled;
        bool export_model;
        bool import_model;
        bool new_object;
        int mode;
    }_param;

	Main()
	{
        ros::param::get("/global_namespace",_param.global_namespace);
		tld = new tld::TLD();

		printResults = NULL;
		saveDir = ".";
		threshold = 0.5;
		showForeground = 0;

		showTrajectory = false;
		trajectoryLength = 0;

		selectManually = 0;

		initialBB = NULL;
		showNotConfident = true;

		reinit = 0;

		loadModel = false;
		keyboardControl = true;
		exportModelAfterRun = false;
		modelExportFile = "model";
		seed = 0;
		//News
		flag=true;
		reuseFrameOnce = false;
		skipProcessingOnce = false;
		resultsFile = NULL;
        obj_select_sub_ = pnode.subscribe("/"+_param.global_namespace+"/object/obj_select", 1, &Main::objSelectCb, this);
		if(selection == NULL){
			selection = new int[4];
		}
		selection[0]=-1;selection[1]=-1;selection[2]=-1;selection[3]=-1;
        _parameter_updated_sub = pnode.subscribe("parameter_updated",10,&Main::paramUpdatedCb,this);
        _param.clear_model = false;
        _param.learning_disabled = false;
        _param.detector_disabled = false;
        _param.export_model = false;
        _param.import_model = false;
        _param.new_object = false;
        _param.mode = 2;


        std::string param_val;

        if(ros::param::has("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_clear_model")){
            ros::param::get("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_clear_model",param_val);
            _param.clear_model = std::stoi(param_val);
        }
        else
            ros::param::set("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_clear_model",std::to_string(_param.clear_model));

        if(ros::param::has("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_learning_disabled")){
            ros::param::get("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_learning_disabled",param_val);
            _param.learning_disabled = std::stoi(param_val);
        }
        else
            ros::param::set("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_learning_disabled",std::to_string(_param.learning_disabled));

        if(ros::param::has("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_detector_disabled")){
            ros::param::get("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_detector_disabled",param_val);
            _param.detector_disabled = std::stoi(param_val);
        }
        else
            ros::param::set("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_detector_disabled",std::to_string(_param.detector_disabled));

        if(ros::param::has("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_export_model")){
            ros::param::get("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_export_model",param_val);
            _param.export_model = std::stoi(param_val);
        }
        else
            ros::param::set("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_export_model",std::to_string(_param.export_model));

        if(ros::param::has("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_import_model")){
            ros::param::get("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_import_model",param_val);
            _param.import_model = std::stoi(param_val);
        }
        else
            ros::param::set("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_import_model",std::to_string(_param.import_model));

        if(ros::param::has("/"+_param.global_namespace+"/parameters/flyt/ob_track_mode")){
            ros::param::get("/"+_param.global_namespace+"/parameters/flyt/ob_track_mode",param_val);
            _param.mode = std::stoi(param_val);
        }
        else
            ros::param::set("/"+_param.global_namespace+"/parameters/flyt/ob_track_mode",std::to_string(_param.mode));

        if(ros::param::has("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_new_object")){
            ros::param::get("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_new_object",param_val);
            _param.new_object = std::stoi(param_val);
        }
        else
            ros::param::set("/"+_param.global_namespace+"/parameters/flyt/ob_track_tld_new_object",std::to_string(_param.mode));

		//image_pub_ = it_.advertise("/"+global_namespace+"/object/object_image", 1);
	}

    ~Main()
    {
        delete tld;
    }
    
    void doWork(const sensor_msgs::ImageConstPtr& msg, const sensor_msgs::PointCloud2ConstPtr& cloudy);
    void doWork(const sensor_msgs::ImageConstPtr& msg);
    void publish(cv::Rect *currBB);
    void loadRosparam();
    void Gui(Mat& img, Mat& grey);
    bool add(open_tld_3d::model::Request  &req, open_tld_3d::model::Response &res);
    void objSelectCb(const std_msgs::Int32MultiArray::ConstPtr& obj);
    void paramUpdatedCb(const std_msgs::StringConstPtr &param_name);
};

#endif /* MAIN_H_ */
