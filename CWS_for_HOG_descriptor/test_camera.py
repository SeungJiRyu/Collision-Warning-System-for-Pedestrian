import cv2

def main():
    cam = cv2.VideoCapture(0,cv2.CAP_V4L)
    cam.set(cv2.CAP_PROP_FRAME_WIDTH,640)
    cam.set(cv2.CAP_PROP_FRAME_HEIGHT,480)
    
    while True:
        ret, image = cam.read()
        cv2.imshow('camera test',image)
        
        if cv2.waitKey(1) > 0 :
            break
        
    cv2.destroyAllWindows()


main()


        
