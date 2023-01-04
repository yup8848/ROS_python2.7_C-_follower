#include "motionController.h"
#include <tesseract/baseapi.h>
#include <tesseract/publictypes.h>

using namespace cv;
using namespace std;

extern CSystemData g_sysData;

void CMotionController::Init(ros::Publisher &rosPublisher)
{
   m_rosPub = rosPublisher;
   thread *pth = new thread(&CMotionController::HandlerController, this);
   m_treads.push_back(pth);

   sMindStateDataType mindState;
   g_sysData.getData(mindState);
   mindState.eMindStateMachine = SM_MANUAL;
   g_sysData.setData(mindState);
}

void CMotionController::SetMotionStatus(motionStatus eStatus)
{
   lock_guard<mutex> lock(lock_m_eMotionStatus);
   m_eMotionStatus = eStatus;
}

int CMotionController::HandlerController()
{
   while (1)
   {
      sMindStateDataType mindState;
      g_sysData.getData(mindState);
      switch (mindState.eMindStateMachine)
      {
      case SM_MANUAL:
      {
         switch (m_eMotionStatus)
         {
         case RUNNING:
         {
            sleep(0.2);
            //   lock_guard<mutex> lock(lock_m_vecMatImage);
            //   int iSize =  m_vecMatImage.size();
            //   for(int i=0;i< iSize;i++)
            //   {
            //      geometry_msgs::Twist   msgs = calc_target(m_vecMatImage[i]);
            //      m_vecMatImage.erase(m_vecMatImage.begin()+i);
            //      --iSize;
            //      --i;
            //      move(msgs);
            //   }
            lock_guard<mutex> lock(lock_m_vecMatImage);
            int iSize = m_vecErrorDistance.size();
            double dAllError;
            for (int i = 0; i < iSize; i++)
            {
               // geometry_msgs::Twist   msgs = Pid(m_vecErrorDistance[i]);
               dAllError = dAllError + m_vecErrorDistance[i];
               // --iSize;
               // --i;
            }
            m_vecErrorDistance.clear();
            geometry_msgs::Twist msgs = Pid(dAllError / iSize);
            move(msgs);
         }
         break;
         case START:
         {
            m_eMotionStatus = RUNNING;
         }
         break;
         case STOP:
         {
            lock_guard<mutex> lock(lock_m_vecMatImage);
            m_vecMatImage.clear();
            sleep(0.01);
         }
         break;
         case UP_CONTINUE: // 向前连续运动
         {
            sleep(0.1);
            double dDistance;
            GetStepDistance(dDistance);
            geometry_msgs::Twist msgs;
            msgs.linear.x = dDistance;
            move(msgs);
            cout << "向前连续运动:"
                 << "linear.x:" << msgs.linear.x << " angular.z:" << msgs.angular.z << endl;
         }
         break;
         case DOWN_CONTINUE: // 向后连续运动
         {
            sleep(0.1);
            double dDistance;
            GetStepDistance(dDistance);
            geometry_msgs::Twist msgs;
            msgs.linear.x = -dDistance;
            move(msgs);
            cout << "DOWN:"
                 << "linear.x:" << msgs.linear.x << " angular.z:" << msgs.angular.z << endl;
         }
         break;
         case TURN_LEFT: // 连续的向左转向
         {
            sleep(0.1);
            double dDistance;
            GetStepDistance(dDistance);
            geometry_msgs::Twist msgs;
            msgs.angular.z = dDistance;
            move(msgs);
         }
         break;
         case TURN_RIGHT: // 连续的向右转向
         {
            sleep(0.1);
            double dDistance;
            GetStepDistance(dDistance);
            geometry_msgs::Twist msgs;
            msgs.angular.z = -dDistance;
            move(msgs);
         }
         break;
         default:
            sleep(0.01);
            break;
         }
         break;
      }
      case SM_AUTO_ROBOT_NAVIGATION:
      {
         lock_guard<mutex> lock(lock_m_vecMatImage);
         int iSize = m_vecMatImage.size();
         for (int i = 0; i < iSize; i++)
         {
            // cout << "calc_target" << endl;
            geometry_msgs::Twist msgs; 
            msgs = calc_target(m_vecMatImage[i]);
            m_vecMatImage.erase(m_vecMatImage.begin() + i);
            --iSize;
            --i;
            if(msgs.angular.z != 0 || msgs.linear.x !=0)
            {
                  move(msgs);
            }
           
          }
           break;
      }
      default:
         break;
      }
   }
}

motionStatus CMotionController::GetMotionStatus()
{
   lock_guard<mutex> lock(lock_m_eMotionStatus);
   return m_eMotionStatus;
}

Mat CMotionController::selectROI(Mat &frame)
{
   int rows = frame.rows;
   int cols = frame.cols;

   Point points[1][4];
   points[0][0] = Point(cols * 0.05, rows);
   points[0][1] = Point(cols * 0.4, rows * 0.4);
   points[0][2] = Point(cols * 0.6, rows * 0.4);
   points[0][3] = Point(cols * 0.95, rows);

   Mat roiImg = Mat::zeros(frame.size(), frame.type());
   const Point *ppt[1] = {points[0]};
   int npt[] = {4};
   fillPoly(roiImg, ppt, npt, 1, Scalar(255, 255, 255), 8);
   return roiImg;
}

geometry_msgs::Twist CMotionController::calc_target(Mat &frame)
{

   Mat res;
   Mat img = frame;
   resize(img, img, Size(800, 600));
   Mat ROIimg = selectROI(img);
   bitwise_and(img, ROIimg, res);

   Mat hsv_img;
   cvtColor(res, hsv_img, COLOR_BGR2HSV);

   Scalar scalarL = Scalar(70, 45, 100);
   Scalar scalarH = Scalar(99, 255, 200);
   Mat mask;
   inRange(hsv_img, scalarL, scalarH, mask);

   Mat GrayImg, GBlurImg, CannyImg;

   GaussianBlur(mask, GBlurImg, Size(3, 3), 1);

   Moments mu = moments(mask, false);
    Point2f mc;
   if(  mu.m00 != 0)   
   {
       mc = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
   }
   else
   {
      geometry_msgs::Twist msg;
      msg.angular.z = 0;
      msg.linear.x  = 0;
      return  msg;
   }
   circle(img, mc, 20, Scalar(0, 0, 255), -1, 8, 0);
   //imshow("img", img);
   //imshow("mask", mask);
   char *outText;
   tesseract::TessBaseAPI api;

   if (api.Init(NULL, "eng"))
   {
      cout << "Error: cannot initialize tesseract." << endl;
      sMindStateDataType mindState;
      g_sysData.getData(mindState);
      mindState.eMindStateMachine = SM_ERROR;
      g_sysData.setData(mindState);
   }

   api.SetImage((uchar *)mask.data, 800, 600, 1, 800);

   outText = api.GetUTF8Text();
   if (outText == nullptr)
   {
      cout << "No Data" << endl;
   }
   else
   {
      char *pt = outText;
      while (*pt)
      {
         if ('A' <= *pt && *pt <= 'Z')
         {
            *pt += 'a' - 'A';
         }
         pt++;
      }
      if (strstr(outText, "stop") != NULL)
      {
         cout << "Found STOP!!" << endl;
         sMindStateDataType mindState;
         g_sysData.getData(mindState);
         mindState.eMindStateMachine = SM_MANUAL;
         g_sysData.setData(mindState);
      }
      else
      {
         // cout << "outText:" << outText << endl;
      }
   }

   api.End();
   delete[] outText;

   // Mat img = frame;
   // resize(img, img, Size(800, 600));
   // Mat hsv_img;
   // cvtColor(img, hsv_img, COLOR_BGR2HSV);
   // blur(hsv_img, hsv_img, Size(3, 3));
   // Scalar scalarL = Scalar(35, 45, 46);
   // Scalar scalarH = Scalar(99, 255, 255);
   // Mat mask;
   // inRange(hsv_img, scalarL, scalarH, mask);
   // Mat res;
   // bitwise_and(img, img, res, mask);

   // Moments mu = moments(mask, false);
   // Point2f mc = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
   // circle(img, mc, 20, Scalar(0, 0, 255), -1, 8, 0);
   // namedWindow("view_mask", CV_WINDOW_AUTOSIZE);
   // imshow("view_mask", mask);
   // namedWindow("view", CV_WINDOW_AUTOSIZE);
   // imshow("view", img);

   double error = mc.x - 400; // 400=width(800)/2calc_target
   static double last_error = 0;
   double Kp = 0.01;
   double Kd = 0;

   geometry_msgs::Twist msg;
   double dStep;
   GetStepDistance(dStep);
   msg.linear.x = dStep;
   //  msg.angular.z = -Kp * (error + Kd * (error - last_error))*0.1;
   msg.angular.z = -m_dPidKp * (error + m_dPidKd * (error - last_error)) * 0.1;
   
   last_error = error;
   cout << "Sending velocity command:"
        <<"mc.x "<<mc.x << "m_dPidKp:" << m_dPidKp <<" m_dPidKd"<<m_dPidKd<<" error:" <<error<<"last_error"<<last_error << endl;
   cout << "Sending velocity command:"
        << "linear.x:" << msg.linear.x << " angular.z:" << msg.angular.z << endl;
   return msg;
}

geometry_msgs::Twist CMotionController::Pid(double dError)
{
   static double last_error = 0;
   //  double Kp=0.01;
   //  double Kd=0;

   geometry_msgs::Twist msg;
   double dStep;
   GetStepDistance(dStep);
   msg.linear.x = dStep;
   msg.angular.z = -m_dPidKp * (dError + m_dPidKd * (dError - last_error)) * 0.1;
   last_error = dError;
   // cout<<"Sending velocity command:" << "linear.x:"<<msg.linear.x<< " angular.z:"<<msg.angular.z<<endl;
   return msg;
}

void CMotionController::addMatImage(Mat mat)
{
   lock_guard<mutex> lock(lock_m_vecMatImage);
   m_vecMatImage.push_back(mat);
}

void CMotionController::addErrorDistance(double dError)
{
   lock_guard<mutex> lock(lock_m_vecErrorDistance);
   m_vecErrorDistance.push_back(dError);
}
void CMotionController::SetMaxLinearX(double x)
{
   lock_guard<mutex> lock(lock_m_dMaxLinearX);
   m_dMaxLinearX = x;
}

void CMotionController::SetMoveUp(double dDistance, bool bContinuous)
{
   if (false == bContinuous)
   {
      geometry_msgs::Twist target;
      target.linear.x = dDistance;
      move(target);
      cout << "向UP运动:"
           << "target.x:" << target.linear.x << " angular.z:" << target.angular.z << endl;
   }
   else
   {
      SetStepDistance(dDistance);
      SetMotionStatus(UP_CONTINUE);
   }
}

void CMotionController::SetMoveDown(double dDistance, bool bContinuous)
{
   if (false == bContinuous)
   {
      geometry_msgs::Twist target;
      target.linear.x = -dDistance;
      move(target);
      cout << "向后运动:"
           << "linear.x:" << target.linear.x << " angular.z:" << target.angular.z << endl;
   }
   else
   {
      SetStepDistance(dDistance);
      SetMotionStatus(DOWN_CONTINUE);
   }
}

void CMotionController::SetTurnLeft(double dAngle, bool bContinuous)
{
   if (false == bContinuous)
   {
      geometry_msgs::Twist target;
      target.angular.z = dAngle;
      move(target);
   }
   else
   {
      SetMotionStatus(TURN_LEFT);
      SetStepDistance(dAngle);
   }
}

void CMotionController::SetTurnRight(double dAngle, bool bContinuous)
{
   if (false == bContinuous)
   {
      geometry_msgs::Twist target;
      target.angular.z = -dAngle;
      move(target);
   }
   else
   {
      SetMotionStatus(TURN_RIGHT);
      SetStepDistance(dAngle);
   }
}

void CMotionController::SetStepDistance(const double dDistance)
{
   lock_guard<mutex> lock(lock_m_dStepDistance);
   m_dStepDistance = dDistance;
}

void CMotionController::GetStepDistance(double &dDistance)
{
   lock_guard<mutex> lock(lock_m_dStepDistance);
   dDistance = m_dStepDistance;
}

void CMotionController::SetPidKp(const double dPidKp)
{
   m_dPidKp = dPidKp;
}

void CMotionController::SetPidKd(const double dPidKd)
{
   m_dPidKd = dPidKd;
}

int CMotionController::move(geometry_msgs::Twist target)
{
   if (target.linear.x >= m_dMaxLinearX)
   {
      target.linear.x = m_dMaxLinearX;
   }
   m_rosPub.publish(target);
}

int CMotionController::Start()
{
   SetMotionStatus(START);
}

int CMotionController::Stop()
{
   SetMotionStatus(STOP);
}
