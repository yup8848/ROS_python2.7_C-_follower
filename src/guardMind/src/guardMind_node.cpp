#include <ros/ros.h>
// #include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
// #include <opencv2/core/core.hpp>
// #include <opencv2/imgproc/imgproc.hpp>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/image_encodings.h>
#include <iostream>
// #include  <Python.h>
#include <python2.7/Python.h>
#include "std_msgs/String.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/Imu.h"
#include "diagnostic_msgs/DiagnosticArray.h"
#include "SocketCore/Socket.h"
#include "Common/Common.h"
#include "Common/ErrorCodeDef.h"
#include "Common/CommonDef.h"
#include "Common/Log.h"
#include "SocketCore/MessageHandler.h"
#include "Common/Config.h"
#include "Common/ipconfig.h"
#include "motionController.h"
#include "systemdata.h"
#include "protocol.h"
using namespace std;
using namespace cv;

// socket通信处理
CMessageHandler m_messHandler;
// CMotionController运动控制
CMotionController g_motionController;
extern CSystemData g_sysData;
void setMotor(geometry_msgs::Twist &msg)
{
}
// 让string支持switch操作作的映射
std::map<std::string, int> m_mapDiagnostics{
    {"Voltage (V)", 1},
    {
        "Percent (%)",
        2,
    },
    {"Capacity (Ah)", 3},
    {"Left Motor Current (A)", 4},
    {"Right Motor Current (A)", 5},
    {"Left Motor Encoder Counts", 6},
    {"Right Motor Encoder Counts", 7},
    {"Controller Temp (C)", 8}};

void diagnosticsCallback(const diagnostic_msgs::DiagnosticArray &msg)
{

  sMindStateDataType sMindStateData;
  g_sysData.getData(sMindStateData);
  for (int i = 0; i < msg.status.size(); i++)
  {
    for (int j = 0; j < msg.status[i].values.size(); j++)
    {
      switch (m_mapDiagnostics[msg.status[i].values[j].key])
      {
      case 1:
        sMindStateData.fVoltage = stod(msg.status[i].values[j].value);
        break;
      case 2:
        sMindStateData.fPrecent = stod(msg.status[i].values[j].value);
        break;
      case 3:
        sMindStateData.fCapcity = stod(msg.status[i].values[j].value);
        break;
      case 4:
        sMindStateData.fLeftMotorCurrent = stod(msg.status[i].values[j].value);
        break;
      case 5:
        sMindStateData.fRightMotorCurrent = stod(msg.status[i].values[j].value);
        break;
      case 6:
        sMindStateData.iLeftMotorEncoderCounts = stoi(msg.status[i].values[j].value);
        break;
      case 7:
        sMindStateData.iRightMotorEncoderCounts = stoi(msg.status[i].values[j].value);
        break;
      case 8:
        sMindStateData.iControllerTemp = stoi(msg.status[i].values[j].value);
        break;
      default:
        break;
      }
    }
  }
  g_sysData.setData(sMindStateData);
  if (!CSocket::GetInstance()->IsConnected("192.168.1.127", "8000"))
  {
    CProtocol::GetInstance()->sendProtocol(PROTOCOL_MINDTODISPLAY, CMD_ROBOTS_STATE_DATA);
    // CSocket::GetInstance()->Connect("192.168.1.127", "8000", "ID", SOCKET_TCP, PROTOCOL_MINDTODISPLAY, 300);
    //  int iLength;
    //  char *pt = m_messHandler.writeDesDataToCharData(CMD_ROBOTS_STATE_DATA,iLength,sMindStateData);
    // CSocket::GetInstance()->WriteFrame("192.168.1.127", "8000", pt, iLength);
    // delete pt;
  }
}

void imuCallback(const sensor_msgs::Imu &msg)
{
  sRobotsImuDataType sRobotsImuData;
  g_sysData.getData(sRobotsImuData);
  sRobotsImuData.fOrientation[0] = msg.orientation.x;
  sRobotsImuData.fOrientation[1] = msg.orientation.y;
  sRobotsImuData.fOrientation[2] = msg.orientation.z;
  sRobotsImuData.fOrientation[3] = msg.orientation.w;
  sRobotsImuData.fOrientation_covariance[0] = msg.orientation_covariance[0];
  sRobotsImuData.fOrientation_covariance[1] = msg.orientation_covariance[1];
  sRobotsImuData.fOrientation_covariance[2] = msg.orientation_covariance[2];
  sRobotsImuData.fOrientation_covariance[3] = msg.orientation_covariance[3];
  sRobotsImuData.fOrientation_covariance[4] = msg.orientation_covariance[4];
  sRobotsImuData.fOrientation_covariance[5] = msg.orientation_covariance[5];
  sRobotsImuData.fOrientation_covariance[6] = msg.orientation_covariance[6];
  sRobotsImuData.fOrientation_covariance[7] = msg.orientation_covariance[7];
  sRobotsImuData.fOrientation_covariance[8] = msg.orientation_covariance[8];
  sRobotsImuData.fAngular_velocity[0] = msg.angular_velocity.x;
  sRobotsImuData.fAngular_velocity[1] = msg.angular_velocity.y;
  sRobotsImuData.fAngular_velocity[2] = msg.angular_velocity.z;
  sRobotsImuData.fAngular_velocity_covariance[0] = msg.angular_velocity_covariance[0];
  sRobotsImuData.fAngular_velocity_covariance[1] = msg.angular_velocity_covariance[1];
  sRobotsImuData.fAngular_velocity_covariance[2] = msg.angular_velocity_covariance[2];
  sRobotsImuData.fAngular_velocity_covariance[3] = msg.angular_velocity_covariance[3];
  sRobotsImuData.fAngular_velocity_covariance[4] = msg.angular_velocity_covariance[4];
  sRobotsImuData.fAngular_velocity_covariance[5] = msg.angular_velocity_covariance[5];
  sRobotsImuData.fAngular_velocity_covariance[6] = msg.angular_velocity_covariance[6];
  sRobotsImuData.fAngular_velocity_covariance[7] = msg.angular_velocity_covariance[7];
  sRobotsImuData.fAngular_velocity_covariance[8] = msg.angular_velocity_covariance[8];
  sRobotsImuData.fLinear_acceleration_covariance[0] = msg.linear_acceleration_covariance[0];
  sRobotsImuData.fLinear_acceleration_covariance[1] = msg.linear_acceleration_covariance[1];
  sRobotsImuData.fLinear_acceleration_covariance[2] = msg.linear_acceleration_covariance[2];
  sRobotsImuData.fLinear_acceleration_covariance[3] = msg.linear_acceleration_covariance[3];
  sRobotsImuData.fLinear_acceleration_covariance[4] = msg.linear_acceleration_covariance[4];
  sRobotsImuData.fLinear_acceleration_covariance[5] = msg.linear_acceleration_covariance[5];
  sRobotsImuData.fLinear_acceleration_covariance[6] = msg.linear_acceleration_covariance[6];
  sRobotsImuData.fLinear_acceleration_covariance[7] = msg.linear_acceleration_covariance[7];
  sRobotsImuData.fLinear_acceleration_covariance[8] = msg.linear_acceleration_covariance[8];
  g_sysData.setData(sRobotsImuData);
  CProtocol::GetInstance()->sendProtocol(PROTOCOL_MINDTODISPLAY, CMD_ROBOTS_IMU_DATA);
  //         if (!CSocket::GetInstance()->IsConnected("192.168.1.127", "8000"))
  //  {
  //        CSocket::GetInstance()->Connect("192.168.1.127", "8000", "ID", SOCKET_TCP, PROTOCOL_GUARD_MIND, 300);
  //  }
  //  int iLength;
  //  char *pt = m_messHandler.writeDesDataToCharData(ROBOTS_IMU_DATA,iLength,sRobotsImuData);
  //  CSocket::GetInstance()->WriteFrame("192.168.1.127", "8000", pt, iLength);
  //  delete pt;
}

void errorDistanceCallback(const geometry_msgs::Twist &msg)
{
  g_motionController.addErrorDistance(msg.angular.z);
}

geometry_msgs::Twist calc_target(Mat &frame)
{
  Mat img = frame;
  resize(img, img, Size(800, 600));
  Mat hsv_img;
  cvtColor(img, hsv_img, COLOR_BGR2HSV);
  blur(hsv_img, hsv_img, Size(3, 3));
  Scalar scalarL = Scalar(0, 0, 0);
  Scalar scalarH = Scalar(180, 255, 46);
  Mat mask;
  inRange(hsv_img, scalarL, scalarH, mask);
  Mat res;
  bitwise_and(img, img, res, mask);

  Moments mu = moments(mask, false);
  Point2f mc = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
  // circle(img, mc,20, Scalar(0,0,255), -1,8,0);
  // namedWindow("view_mask", CV_WINDOW_AUTOSIZE);
  // imshow("view_mask", img);

  double error = mc.x - 400; // 400=width(800)/2
  static double last_error = 0;
  double Kp = 0.01;
  double Kd = 0;

  geometry_msgs::Twist msg;
  msg.linear.x = 0.1;
  msg.angular.z = Kp * (error + Kd * (error - last_error));
  last_error = error;
  cout << "Sending velocity command:"
       << "linear.x:" << msg.linear.x << " angular.z:" << msg.angular.z << endl;
  return msg;
}

void imageCallback(const sensor_msgs::ImageConstPtr &msg)
{
  // ROS_INFO("I recieve an image.");
  try
  {
    // cout << "imageCallback." << endl;
    // imshow("view", cv_bridge::toCvShare(msg, "bgr8"calc_target)->image);
    Mat img = cv_bridge::toCvShare(msg, "bgr8")->image;
    g_motionController.addMatImage(img);

    // namedWindow("view", CV_WINDOW_AUTOSIZE);
    // imshow("view", img);
    // cvtColor(img, hsv_img, COLOR_BGR2HSV);
    // blur(hsv_img, hsv_img, Size(3,3));
    // Scalar scalarL = Scalar(0,0,0);
    // Scalar scalarH = Scalar(180,255,46);
    // Mat mask;
    // inRange(hsv_img, scalarL, scalarH, mask);
    // Mat res;
    // bitwise_and(img, img, res, mask);
    /*  resize(img, img, Size(800,600));
    Mat hsv_img;*/
    // Moments mu = moments(mask, false);ROBOTS_IMU_DATA
    // Point2f mc = Point2f( mu.m10/mu.m00, mu.m01/mu.m00);
    // circle(img, mc,20, Scalar(0,0,255), -1,8,0);
    // line(img,Point(25,10),Point(10,10), Scalar(0,0,255), 5);
    // namedWindow("view_mask", CV_WINDOW_AUTOSIZE);
    // imshow("view_mask", img);
    //  geometry_msgs::Twist geo_msg;CMotionController *m_motionController  =  new CMotionControROBOTS_IMU_DATAller;
    // geo_msg = calc_target(img);
    // ///////////////////////////////////
    /*
    Mat canny_output;
    vector<vector<Point>> contours;calc_target
    vector<Vec4i> hierarchy;
    double thresh = 2.0;
    Canny( mask, canny_output, thresh, thresh*2);
   // namedWindow("view_Canny", CV_WINDOW_AUTOSIZE);
   // imshow("view_Canny", canny_output);find
           std::lock_guard<std::mutex> lock(lock_m_bulldogDriver);
    findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
    vector<Moments> mu(contours.size());
    for(int i=0; i<contours.size(); i++){calc_target
      mu[i] = moments(contours[i], false);
    }
    vector<Point2f> mc(contours.size());
    for(int i=0; i<contours.size(); i++){
      mc[i] = Point2f( mu[i].m10/mu[i].m00, mu[i].m01/mu[i].m00);
    }
calc_target
    imshow("view_Contours", drawing);*/
    /* waitKey(5);*/
  }
  catch (cv_bridge::Exception &e)
  {
    ROS_ERROR("could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
  }
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "guardMind");
  ros::NodeHandle nh;
  // Initialization for robot motor-control
  ros::Publisher pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 1000);
  ros::Rate loop_rate(10);

  CSocket::GetInstance()->RegMessageHandler(&m_messHandler);
  CSocket::GetInstance()->Start();
  SocketServerListener(8000, SOCKET_TCP, PROTOCOL_DISPLAYTOMIND);

  string strPath = GetCurDirectory();
  string strLogDir = GetCurDirectory() + GetDirectorySpace() + "GMS_LOG" + GetDirectorySpace() + "GuardMindServer" + GetDirectorySpace();
  SetLogFileMaxCount(CGmsConfig::GetInstance()->GetConfig().logFileCount);
  log_set_param(CGmsConfig::GetInstance()->GetConfig().logFileSwitch, CGmsConfig::GetInstance()->GetConfig().logFileSwitch);
  InitLog(strLogDir, GetTimeStampSecond());

  ros::Subscriber subDiagnosticMats = nh.subscribe("/diagnostics", 1, diagnosticsCallback);
  ros::Subscriber subImu = nh.subscribe("/imu/data", 1, imuCallback);
  ros::Subscriber subYu = nh.subscribe("/yu", 1, errorDistanceCallback);
  //  Initialization using Realsense_Camera

  // //  namedWindow("view", CV_WINDOW_NORMAL);
  // image_transport::ImageTransport it(nh);
  // image_transport::Subscriber sub = it.subscribe("/camera/color/image_raw", 1, imageCallback);

  // Initialization using USB_Camera
    VideoCapture capture;
    capture.open("/dev/video0");
     if(!capture.isOpened()){
        cout<<"failed to open camera."<<endl;         
      }else{
        cout<<"succeed to open camera."<<endl;
      }

  // 未初始化

   g_motionController.Init(pub);
   g_motionController.SetMotionStatus(STOP);

  // Size size = Size(capture.get(CV_CAP_PROP_FRAME_WIDTH), capture.get(CV_CAP_PROP_FRAME_HEIGHT));
  // VideoWriter writer;
  // writer.open("final.avi", CV_FOURCC('M','J','P','G'),10,size, true);


  // geometry_msgs::Twist msg;
  // msg.linear.x = 0.1;Common
  // msg.angular.z = 1;
     Mat picture ;
   namedWindow("view_mask", CV_WINDOW_AUTOSIZE);
  // imshow("view_mask", img);
    //  while (capture.read(picture))
    //  {
    //      imshow("view_mask", picture);
         
        
    //  }

   while (ros::ok())
   {
     //  capture.read(frame);
  //   //  geo_msg = calc_target(frame);
  //   //  pub.publish(geo_msg);
     capture.read(picture);
    if(!picture.empty())
    {
       g_motionController.addMatImage(picture);
    }
      //   // CProtocol::GetInstance()->sendProtocol(PROTOCOL_MINDTODISPLAY, CMD_ROBOTS_MOVE);

     waitKey(3);
     ros::spinOnce();
     loop_rate.sleep();
   }
  // // ros::spin();
  // cv::destroyWindow("view");
}

