import cv2
import mediapipe as mp
from mediapipe.tasks.python import vision
import socket
import struct

# mediapipe hand landmarker model & setting
model_path = "models/hand_landmarker.task"

base_options = mp.tasks.BaseOptions(model_asset_path = model_path)
option = vision.HandLandmarkerOptions(
    base_options = base_options,
    num_hands = 1,
    running_mode = vision.RunningMode.VIDEO,
    )
landmarker = vision.HandLandmarker.create_from_options(option)

# socket setting
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_address = ("127.0.0.1", 5005) # 自分のパソコン(127.0.0.1)の 5005ポート


cap = cv2.VideoCapture(0)
width = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
height = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
timestamp_ms = 0

cam_size = struct.pack("ff", cap.get(cv2.CAP_PROP_FRAME_WIDTH), cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
sock.sendto(cam_size, server_address)

while cap.isOpened():
    ret, frame = cap.read()
    if not ret: break

    points = []

    frame = cv2.flip(frame, 1)
    rgb = cv2.cvtColor(frame,cv2.COLOR_BGR2RGB)
    mp_imgae = mp.Image(image_format=mp.ImageFormat.SRGB, data = rgb)
    fps = cap.get(cv2.CAP_PROP_FPS)
    timestamp_ms += int(1000/fps)
    result = landmarker.detect_for_video(mp_imgae, timestamp_ms)

    if result.hand_landmarks:
        for landmarks in result.hand_landmarks:
            for i in [4,8,12,16,20]:
                cx, cy = landmarks[i].x, landmarks[i].y
                points.append(cx)
                points.append(cy)
        data = struct.pack("10f",*points)
        sock.sendto(data, server_address)
        
cap.release()
cv2.destroyAllWindows()