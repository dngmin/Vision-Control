import cv2
import mediapipe as mp
from mediapipe.tasks.python import vision
import pyautogui
from math import *

#Const
Sensitivity = 3

def get_ratio(cap):
    #webcam size
    C_width, C_height = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH)), int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    #Monitor size
    M_width, M_height = pyautogui.size()
    #monitor/webcam ratio
    R_width, R_height = int(M_width/C_width), int(M_height/C_height)
    if R_height > R_width:
        Ratio = R_height
    else:
        Ratio = R_width
    return Ratio

def hand_tremor_compensation(prev_x,prev_y,curr_x,curr_y,frame,ratio,sens = Sensitivity):
    dxy = sqrt((prev_x-curr_x)**2 + (prev_y-curr_y)**2)
    if dxy < sens:
        pass
    else:
        cv2.circle(frame,(curr_x,curr_y),3,(0,255,0),-1)
        pyautogui.moveTo(curr_x*ratio,curr_y*ratio,duration=0.1)

def gesture_detection(fingertip_points):
    gesture = False
    try:
        #0:Thumb 1:Middle 2:Ring 3:Pinky
        thumb = fingertip_points[0]
        middle = fingertip_points[1]
        ring = fingertip_points[2]
        pinky = fingertip_points[3]

        thumb_to_middle = int(sqrt((thumb[0]-middle[0])**2 + (thumb[1]-middle[1])**2))
        middle_to_ring = int(sqrt((middle[0]-ring[0])**2 + (middle[1]-ring[1])**2))
        ring_to_pinky = int(sqrt((ring[0]-pinky[0])**2 + (ring[1]-pinky[1])**2))

        t_m_ratio = thumb_to_middle/middle_to_ring
        r_p_ratio = ring_to_pinky/middle_to_ring

        if t_m_ratio > 3 and r_p_ratio > 3:
            gesture = "D"
        elif t_m_ratio > 3:
            gesture = "L"
        elif r_p_ratio > 3:
            gesture = "R"
    except:
        pass
    return gesture

px = False
py = False
cx = False
cy = False

model_path = 'model/hand_landmarker.task'

base_options = mp.tasks.BaseOptions(model_asset_path=model_path)
option = vision.HandLandmarkerOptions(
    base_options = base_options,
    num_hands = 1,
    running_mode = vision.RunningMode.VIDEO,
    )
landmarker = vision.HandLandmarker.create_from_options(option)

cap = cv2.VideoCapture(0)
ratio = get_ratio(cap)

timestamp_ms = 0

while cap.isOpened():
    ret, frame = cap.read()

    if not ret:
        break
    fliped = cv2.flip(frame,1)
    rgb = cv2.cvtColor(fliped,cv2.COLOR_BGR2RGB)
    mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data = rgb)

    fps = cap.get(cv2.CAP_PROP_FPS)
    timestamp_ms += int(1000/fps)
    result = landmarker.detect_for_video(mp_image, timestamp_ms)

    #0:Thumb 1:Middle 2:Ring 3:Pinky
    fingertip_points = []

    if result.hand_landmarks:
        for landmarks in result.hand_landmarks:

            cx, cy = int(landmarks[8].x * frame.shape[1]), int(landmarks[8].y * frame.shape[0])
            hand_tremor_compensation(px,py,cx,cy,fliped,ratio)
            #print(cx,cy)

            for i in (4,12,16,20):
                ix, iy = int(landmarks[i].x * frame.shape[1]), int(landmarks[i].y * frame.shape[0])
                cv2.circle(fliped,(ix,iy),3,(0,255,0),-1)
                fingertip_points.append((ix,iy))

    gesture = gesture_detection(fingertip_points)
    if gesture == False:
        pass
    elif gesture == "D":
        pyautogui.doubleClick()
    elif gesture == "L":
        pyautogui.leftClick()
    elif gesture == "R":
        pyautogui.rightClick()

    cv2.imshow("Handmarker", fliped)
    px = cx
    py = cy

    if cv2.waitKey(1) == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()