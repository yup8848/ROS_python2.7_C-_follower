#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy
import rospy
import cv2, cv_bridge
from cv_bridge import CvBridge, CvBridgeError
from sensor_msgs.msg import Image
import numpy as np
from geometry_msgs.msg import Twist
import pytesseract
import rospy, cv2, cv_bridge, numpy
from sensor_msgs.msg import CompressedImage


class Follower:

    def __init__(self):
        self.bridge = cv_bridge.CvBridge()
        # 订阅topic----(/camera/color/image_raw)
        self.image_sub = rospy.Subscriber('/camera/color/image_raw', Image, self.image_callback)
        print("--> __init__(self)")
        self.cmd_vel_pub = rospy.Publisher('yu', Twist, queue_size=1000)
        self.twist = Twist()

    def image_callback(self, msg):
        # ros的msg转换成opencv格式
        try:
            image = self.bridge.imgmsg_to_cv2(msg, 'bgr8')
        except CvBridgeError as e:
            print (e)
        # ROI区域获取
        (b, g, r) = cv2.split(image)  # 通道分解
        bH = cv2.equalizeHist(b)
        gH = cv2.equalizeHist(g)
        rH = cv2.equalizeHist(r)
        result = cv2.merge((bH, gH, rH), )  # 通道合成
        image = result.copy()

        while image is not None:

            custom_config = r'-l eng --oem 3  -c tessedit_char_whitelist=T  --psm 10 '
            code = pytesseract.image_to_string(image, config=custom_config)
            print (code)
            if code == 'T':
                print('congratuation')
                break

            h, w, d = image.shape
            poly = numpy.array([
                [(0, h), (640, h), (340, 180), (300, 180)]])
            black_image = numpy.zeros_like(image)
            mask = cv2.fillPoly(black_image, poly, color=(255, 255, 255))
            #  applying mask on original image
            masked_image = cv2.bitwise_and(image, mask)

            # cv.imshow('win', masked_image)
            image = numpy.copy(masked_image)

            # RGB灰度转化
            hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
            lower_black = numpy.array([35, 43, 46])
            upper_black = numpy.array([77, 255, 255])
            # hsv图像尺寸和通道数
            # print (hsv.shape)
            # 灰度范围取值
            mask = cv2.inRange(hsv, lower_black, upper_black)
            masked = cv2.bitwise_and(image, image, mask=mask)
            res = cv2.cvtColor(masked, cv2.COLOR_BGR2GRAY)
            blur = cv2.medianBlur(res, 3)
            cv2.imshow('media_gray_green', blur)

            dst = cv2.equalizeHist(res)
            # ret, img_thresh = cv2.threshold(dst, 203, 255, cv2.THRESH_BINARY)
            img_thresh = cv2.adaptiveThreshold(res, 255, cv2.ADAPTIVE_THRESH_MEAN_C, cv2.THRESH_BINARY, 9, -10)

            Matrix = numpy.ones((3, 3), numpy.uint8)
            img_edge1 = cv2.erode(img_thresh, Matrix)
            cv2.imshow('erode.jpg', img_edge1)
            Matrix2 = numpy.ones((5, 5), numpy.uint8)
            img_edge2 = cv2.dilate(img_edge1, Matrix2)
            # cv.imwrite('dilate.jpg',img_edge2)
            cv2.imshow('dilate.jpg', img_edge2)

            # 局部图像取值和图像重心计算
            h, w = img_edge2.shape
            # 考虑图像1/2高，30行宽部分
            search_top = 3 * h // 4
            search_bot = search_top + 20
            mask[0:search_top, 0:w] = 0
            mask[search_bot:h, 0:w] = 0
            # 输出重心M
            M = cv2.moments(img_edge2)

            if M['m00'] > 0:
                cx = int(M['m10'] / M['m00'])
                cy = int(M['m01'] / M['m00'])
                cv2.circle(image, (cx, cy), 20, (0, 0, 255), -1)
                err = cx - w / 2
                print (self.twist)
                self.twist.linear.x = 0.0
                self.twist.angular.z = -float(err) / 100
                self.cmd_vel_pub.publish(self.twist)

            # 窗口显示图像
            cv2.namedWindow("window", cv2.WINDOW_NORMAL)
            cv2.imshow('window', image)
            print("got msg, continue...")
            cv2.waitKey(3)
            if 0xFF == ord('q'):
                image.release()
                cv2.destroyAllWindows()


print("--->rospy.init_node")
rospy.init_node('follower', anonymous=True)
# grey_image = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
# image_sub = rospy.Subscriber('/camera/color/image_raw', Image, image_callback)
print("--->follower = Follower()")
follower = Follower()
rospy.spin()
