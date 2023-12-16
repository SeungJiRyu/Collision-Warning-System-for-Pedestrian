import cv2
import RPi.GPIO as GPIO

# Load HOG descriptor
hog = cv2.HOGDescriptor()
hog.setSVMDetector(cv2.HOGDescriptor_getDefaultPeopleDetector())

# Load camera
cap = cv2.VideoCapture(0,cv2.CAP_V4L)
cap.set(cv2.CAP_PROP_FRAME_WIDTH,640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT,480)

# Setting GPIO output
GPIO_PIN = 17
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(GPIO_PIN,GPIO.OUT)

while True:
    GPIO.output(GPIO_PIN,GPIO.LOW)
    ret, frame = cap.read()

    if not ret:
        break

    # Object detection
    detected, _ = hog.detectMultiScale(frame)
    
    # Display bounding boxes around detected objects
    for (x, y, w, h) in detected:
        cv2.rectangle(frame, (x, y, w, h), (0, 255, 0), 3)
        cv2.putText(frame, 'Person',(x, y - 5), cv2.FONT_HERSHEY_DUPLEX, 1, (0,255,0), 1)
        GPIO.output(GPIO_PIN,GPIO.HIGH)

    cv2.imshow('HOG_vedio_test', frame)

    if cv2.waitKey(30) > 0:
        break

cap.release()
cv2.destroyAllWindows()
