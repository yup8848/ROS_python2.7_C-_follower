#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy, cv2, cv_bridge, numpy
from sensor_msgs.msg import CompressedImage
from geometry_msgs.msg import Twist


class Follower:
    def __init__(self):
      self.bridge = cv_bridge.CvBridge()
      self.image_sub = rospy.Subscriber('/camera/color/image_raw/compressed', CompressedImage, self.image_callback)
      self.cmd_vel_pub = rospy.Publisher('yu', Twist, queue_size=1000)
      self.twist = Twist()
      self.bais = [ ]

    def image_callback(self, msg):
        image = self.bridge.compressed_imgmsg_to_cv2(msg, desired_encoding='bgr8')

        h, w, d = image.shape
        poly = numpy.array([
                [(0, h), (640, h), (340, 180), (300, 180)]])
        black_image = numpy.zeros_like(image)
        mask = cv2.fillPoly(black_image, poly, color=(255, 255, 255))
            #  applying mask on original image
        masked_image = cv2.bitwise_and(image, mask)

            # cv.imshow('win', masked_image)
        image = numpy.copy(masked_image)

        hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        lower_yellow = numpy.array([35, 43, 46])
        upper_yellow = numpy.array([77, 255, 255])
        mask = cv2.inRange(hsv, lower_yellow, upper_yellow)
        h, w, d = image.shape
        top = 5 * h / 6
        bot = top + 20
        mask[0:top, :] = 0
        mask[bot:h, :] = 0
        # cut the image to a blade
        M = cv2.moments(mask)
        # class MOMENTS
        if M['m00'] > 0:
            cx = int(M['m10'] / M['m00'])
            cy = int(M['m01'] / M['m00'])
            cv2.circle(image, (cx, cy), 20, (0, 0, 255), -1)
            err = cx - w / 2
            print (self.twist)
            self.twist.linear.x = 0.0
            self.twist.angular.z = -float(err) / 100
            self.cmd_vel_pub.publish(self.twist)
        cv2.namedWindow("window2", cv2.WINDOW_NORMAL)
        cv2.imshow("window2", image)

        cv2.waitKey(3)


rospy.init_node('follower', anonymous=True)
follower = Follower()
rospy.spin()
