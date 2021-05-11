import cv2 as cv
import numpy as np

def detect_red_car(img: np.array) -> np.array:
    """ 检测图片中的🚗，其他颜色找不到~     
    先转成HSV通道，而后分离出红色，注意饱和度和亮度，👨‍🦳     
    Args:
        img: Image of red car
    Returns:    
        Tagged image
    """
    imgcpy = img.copy()
    # 转成HSV通道
    hsvimg = cv.cvtColor(img, cv.COLOR_BGR2HSV)
    # 分出红色，保证饱和度和亮度
    out = cv.inRange(hsvimg, np.array([0, 80, 50]), np.array([9, 255, 220])) | cv.inRange(hsvimg, np.array([160, 80, 50]), np.array([180, 255, 220]))
    # 腐蚀一下，去掉小白点儿
    out = cv.morphologyEx(out, cv.MORPH_ERODE, np.ones((2, 2), np.uint8))
    contours, hierarchy = cv.findContours(out, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_NONE)   # 查找轮廓
    outMask = np.zeros(out.shape, np.uint8)
    _index, _area = -1, 0 
    for i in range(len(contours)):
        area = cv.contourArea(contours[i])
        if area > _area:
            _index = i
            _area = area
    if _index != -1:
        outMask = cv.drawContours(outMask, contours, _index, 255, thickness=2)
    else:
        return None
    x, y, w, h = cv.boundingRect(outMask)
    maskSize = max(w, h) + 20
    #cv.rectangle(img, (x, y), (x + w, y + h), (255, 0, 0), 5)
    
    final_out = cv.bitwise_and(outMask, out)
    final_out = cv.bitwise_not(final_out)
    final_out = final_out[y:y+h, x:x+w]

    final_out = cv.copyMakeBorder(final_out,int((maskSize-h)/2),int((maskSize-h)/2),int((maskSize-w)/2),int((maskSize-w)/2),cv.BORDER_CONSTANT,value=[255,255,255])
    return final_out
    

#img = cv.imread(r'C:\Users\Dragon\Desktop\img2.jpg',cv.IMREAD_COLOR)

# 读取视频帧
# ret, frame = cap.read()
# # 镜像转换
# frame = cv2.flip(frame, 1)
# img1 = detect_red_car(img)
# cv.imshow('img', img)
# cv.waitKey(0)