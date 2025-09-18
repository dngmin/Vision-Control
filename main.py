import cv2
import mediapipe as mp
from mediapipe.tasks.python import vision
import pyautogui


def get_ratio(cap):
    pyautogui.sleep(1)
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

model_path = 'model/hand_landmarker.task'

base_options = mp.tasks.BaseOptions(model_asset_path=model_path)
option = vision.HandLandmarkerOptions(
    base_options = base_options,
    num_hands = 1,
    running_mode = vision.RunningMode.VIDEO,
)
landmarker = vision.HandLandmarker.create_from_options(option)

cap = cv2.VideoCapture(0)
Ratio = get_ratio(cap)

timestamp_ms = 0

while cap.isOpened():
    ret, frame = cap.read()

    if not ret:
        break
    fliped = cv2.flip(frame,1)
    rgb = cv2.cvtColor(fliped,cv2.COLOR_BGR2RGB)
    mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data = rgb)

    timestamp_ms += 33
    result = landmarker.detect_for_video(mp_image, timestamp_ms)

    if result.hand_landmarks:
        for landmarks in result.hand_landmarks:

            cx, cy = int(landmarks[8].x * frame.shape[1]), int(landmarks[8].y * frame.shape[0])
            cv2.circle(fliped, (cx,cy), 3, (0,255,0), -1)
            pyautogui.moveTo(cx*Ratio,cy*Ratio)
        
    cv2.imshow("Handmarker", fliped)
    pyautogui.sleep(0.1)

    if cv2.waitKey(1) == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()