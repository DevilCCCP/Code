#!/bin/bash

avconv -y -rtsp_transport tcp -i rtsp://172.17.1.250:554/11 -an -c copy ./video_001.mp4
