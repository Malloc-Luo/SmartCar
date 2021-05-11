import numpy as np
import cv2 as cv
import math
#from Singlecamconfig import mtx,dist,newcameramtx
from usartWith32 import Usart_Send,Usart_Rece
from CatchPicWithNums import  DetectBall,getBackboard

def nothing(x):
    pass
'''
cv.namedWindow('TestWin', cv.WINDOW_NORMAL)
#Background = np.zeros((0,0,3), np.uint8)
cv.createTrackbar('green_h1', 'TestWin',33,255,nothing)
cv.createTrackbar('green_h2', 'TestWin',93,255,nothing)
cv.createTrackbar('green_s1', 'TestWin',40,255,nothing)
cv.createTrackbar('green_v1', 'TestWin',35,255,nothing)

cv.createTrackbar('red_h1', 'TestWin',20,255,nothing)
cv.createTrackbar('red_h2', 'TestWin',15,255,nothing)
cv.createTrackbar('red_s1', 'TestWin',141,255,nothing)
cv.createTrackbar('red_v1', 'TestWin',46,255,nothing)

cv.createTrackbar('red_2_h1', 'TestWin',150,255,nothing)
cv.createTrackbar('red_2_h2', 'TestWin',180,255,nothing)
cv.createTrackbar('red_2_s1', 'TestWin',130,255,nothing)
cv.createTrackbar('red_2_v1', 'TestWin',46,255,nothing)


cv.createTrackbar('blue_h1', 'TestWin',15,255,nothing)#100
cv.createTrackbar('blue_h2', 'TestWin',155,255,nothing)
cv.createTrackbar('blue_s1', 'TestWin',20,255,nothing)#50
cv.createTrackbar('blue_v1', 'TestWin',30,255,nothing)#25

cv.createTrackbar('num1', 'TestWin',10,100,nothing)
'''

#TODO:写一个界面调试阈值

def cnt_area(cnt):
  area = cv.contourArea(cnt)
  return area

def detectandProcess(img):

    t0 = cv.getTickCount()

    hsv = cv.cvtColor(img, cv.COLOR_BGR2HSV)
    '''
    rh1 = cv.getTrackbarPos('red_h1','TestWin')
    rh2 = cv.getTrackbarPos('red_h2','TestWin')
    rs1 = cv.getTrackbarPos('red_s1','TestWin')
    rv1 = cv.getTrackbarPos('red_v1','TestWin')

    r2h1 = cv.getTrackbarPos('red_2_h1','TestWin')
    r2h2 = cv.getTrackbarPos('red_2_h2','TestWin')
    r2s1 = cv.getTrackbarPos('red_2_s1','TestWin')
    r2v1 = cv.getTrackbarPos('red_2_v1','TestWin')
    '''
    '''
    gh1 = cv.getTrackbarPos('green_h1','TestWin')
    gh2 = cv.getTrackbarPos('green_h2','TestWin')
    gs1 = cv.getTrackbarPos('green_s1','TestWin')
    gv1 = cv.getTrackbarPos('green_v1','TestWin')
    '''
    '''
    bh1 = cv.getTrackbarPos('blue_h1','TestWin')
    bh2 = cv.getTrackbarPos('blue_h2','TestWin')
    bs1 = cv.getTrackbarPos('blue_s1','TestWin')
    bv1 = cv.getTrackbarPos('blue_v1','TestWin')
    '''

    t1 = cv.getTickCount()
    '''
    gh1 = cv.getTrackbarPos('green_h1','TestWin')
    gh2 = cv.getTrackbarPos('green_h2','TestWin')
    gs1 = cv.getTrackbarPos('green_s1','TestWin')
    gv1 = cv.getTrackbarPos('green_v1','TestWin')
    '''
    #print((t1-t0))
    # 定义HSV中蓝色的范围
    lower_blue = np.array([100, 20, 30])
    upper_blue = np.array([155, 255, 255])
    # 设置HSV的阈值使得只取蓝色
    mask_B = cv.inRange(hsv, lower_blue, upper_blue)

    lower_green = np.array([33, 40, 35])
    upper_green = np.array([99, 255, 255])
    mask_G = cv.inRange(hsv, lower_green, upper_green)

    lower_red1 = np.array([0, 141, 46])
    upper_red1 = np.array([15, 255, 255])
    mask_R1 = cv.inRange(hsv, lower_red1, upper_red1)

    lower_red2 = np.array([150, 130, 46])
    upper_red2 = np.array([180, 255, 255])
    mask_R2 = cv.inRange(hsv, lower_red2, upper_red2)

    red_thresh =  mask_R1 + mask_R2
    green_thresh = mask_G
    blue_thresh = mask_B

    cv.imshow('blue_thresh',blue_thresh)
    cv.imshow('green_thresh',green_thresh)
    cv.imshow('red_thresh',red_thresh)

    t2 = cv.getTickCount()



    t3 = cv.getTickCount()


    return red_thresh,green_thresh,blue_thresh



def getContours(img):

    red_closing,green_closing,blue_closing = detectandProcess(img)
    t3 = cv.getTickCount()
    #edges = cv.Canny(red_closing,100,200)
    #cv.imshow('edges',edges)
    _,blue_contours, blue_hierarchy = cv.findContours(blue_closing, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
    _,green_contours, green_hierarchy = cv.findContours(green_closing, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
    _,red_contours, red_hierarchy = cv.findContours(red_closing, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)

    t4 = cv.getTickCount()
    #print((t4-t3))
    red_contours.sort(key=cnt_area, reverse=True) #选择法排序，方便查找最大和最小的
    green_contours.sort(key=cnt_area, reverse=True)
    blue_contours.sort(key=cnt_area, reverse=True)

    t5 = cv.getTickCount()


    new_green_contours=[]
    new_red_contours=[]
    new_blue_contours=[]
    filter_size = 1000
    for it in iter(green_contours):
        #print('size',cv.contourArea(it))
        if cv.contourArea(it) > filter_size:
            new_green_contours.append(it)
            #print('new newnewnewsize',cv.contourArea(it))
            #print(it)
            cv.drawContours(frame,it,-1,(0,255,255),5)

    for it in iter(red_contours):
        #print('size',cv.contourArea(it))
        if cv.contourArea(it) > filter_size:
            new_red_contours.append(it)
            #print('new newnewnewsize',cv.contourArea(it))
            #print(it)
            cv.drawContours(frame,it,-1,(0,255,255),5)

    for it in iter(blue_contours):
        #print('size',cv.contourArea(it))
        if cv.contourArea(it) > filter_size:
            new_blue_contours.append(it)
            #print('new newnewnewsize',cv.contourArea(it))
           #print(it)
            cv.drawContours(frame,it,-1,(0,255,255),5)


        #print('size',it.size())
    #print((t5-t4))

    #if  len(contours)>=0 :
    #    cnt = contours[0]

    return new_red_contours,new_green_contours,new_blue_contours




def filterShapes(frame,shape):

    #print(len(shapes))

    #rh1 = cv.getTrackbarPos('red_h1','TestWin')


    stdShape = []
    center = [0,0]
    edgeLength = 0
    shapeCode = 0

    epsilon = (20/500)*cv.arcLength(shape,True)
    approx = cv.approxPolyDP(shape,epsilon,True)

    M = cv.moments(shape)

    if M['m00'] == 0:
        return False,center,edgeLength,shapeCode

    area = cv.contourArea(shape)
    perimeter = cv.arcLength(shape,True)

    corners = len(approx)
    #print('corners',corners)
    shape_type = ""



    if corners == 3:
        shape_type = "triangle"
        edgeLength = perimeter/3
        shapeCode = 3
        Shape = 3

    if corners == 4:
        shape_type = "rectangle"
        #edgeLength = perimeter/4

        Minrect = cv.minAreaRect(shape)
        box =  cv.boxPoints(Minrect)
        box = np.int0(box)
        cv.drawContours(frame,[box],0,(0,0,255),2)
        edgeLength = math.sqrt(math.pow((box [0][0]-box [1][0]),2)+ math.pow((box [0][0]-box [0][1]),2))
            #print('box',box)
            #cen_x = (box [0][1]+box [1][1]+box [2][1]+box [3][1])/4
            #cen_y = (box [0][0]+box [1][0]+box [2][0]+box [3][0])/4
        Shape = 1
        shapeCode = 1

    if corners >= 5:
            #TODO:detect circles

            #circles1 = cv.HoughCircles(frame,cv.HOUGH_GRADIENT,1,20,param1=50,param2=30,minRadius=0,maxRadius=0)
            #circles1 = np.uint16(np.around(circles1))

        (x,y),radius = cv.minEnclosingCircle(shape)
        #print('radius',radius)
        center = (int(x),int(y))
        radius = int(radius)
        cv.circle(frame,center,radius,(0,255,0),2)
        shape_type = "circle"
        Shape = 2
        shapeCode = 2
        edgeLength = radius * 2




    #print( M )
    cx = int(M['m10']/M['m00'])
    cy = int(M['m01']/M['m00'])
    center = [cx,cy]
    #print('edgeLength',edgeLength)
    #print('center',center)

    delta = 0;
    meanDist = 0;

    cv.putText(frame, shape_type, (cx, cy), cv.FONT_HERSHEY_SIMPLEX,0.5, (255, 255, 255), 2)

    pt = approx[0][0];
    last_dist = math.sqrt((pt[0]-cx)*(pt[0]-cx)+(pt[1]-cy)*(pt[1]-cy))
    meanDist = meanDist + last_dist;
    for pti in range(1,len(approx)):
        pt = approx[pti][0];
        dist = math.sqrt((pt[0]-cx)*(pt[0]-cx)+(pt[1]-cy)*(pt[1]-cy))
        delta = delta + (last_dist - dist) * (last_dist - dist) ;
        meanDist = meanDist + dist;
        #print(pt,'  ',dist)

    delta = math.sqrt(delta)
    meanDist = meanDist / len(approx);

    #print(delta,'  ',meanDist)
    if (delta <15 |corners>4)& (meanDist < 90) &( meanDist > 25):# it usually goes into this judge but not satifies!
        cv.polylines(frame, [approx], True, (255, 0, 0), 2)
        #print('Delta Judege is Trueeeeee')
        return True,center,edgeLength,shapeCode  #when in blue, it didn`t return ,so we should return a value anyway!
    else:
        #print('Delta Judege is Falllllllll')
        #return False,center,edgeLength,shapeCode #return here would be no return if 'if' is not true,thus return nonetype! causing a problem!!
    #print('edgeLength',edgeLength)  #only in 'if' can we get the feedback value!
    #print('center',center)
        return False,center,edgeLength,shapeCode # this return means we catch nothing!

'''
def scanShape(binImg,col,row):
    border = 40

    for i in range(border,row - border,5):
        for j in range(border,col - border,5):

'''


def DoTaskOne():
    global error_x,error_y,discernFlag,realDis
    framecopy = frame.copy()
    redShape,greenShape,blueShape = getContours(framecopy)


    #$#_,bluePt = filterShapes(frame,blueShape);
    shapeCode = 0
    sc = 0
    color = 0;
    fc = [0,0];
    fe = 0;
    b = 0;

    if len(redShape) >0 :
        b,c,e,sc = filterShapes(framecopy,redShape[0])

    if(b):
        color = 1
        fc = c
        fe = e
        shapeCode = sc
        print('firstColoooooooooooooooo')
    else:
        print('Error,RED no return')
    if len(blueShape) >0 :
        b,c,e,sc = filterShapes(framecopy,blueShape[0])

    if(b):
        color = 2
        fc = c
        fe = e
        shapeCode = sc
        print('SecondColrrrrrrrrrrrrrrr')
    else:
        print('Error,BLUE no return')
    if len(greenShape) >0 :
        b,c,e,sc = filterShapes(framecopy,greenShape[0])
    if(b):
        color = 3
        fc = c
        fe = e
        shapeCode = sc
        print('ThirdColdddddddddddddddd')
    else:
        print('Error,GREEN no return')

    pitch = (fc[1] - 120) * 0.1
    yaw = (fc[0]-160) * 0.1
    error_x = yaw
    error_y = pitch
    Color = color
    fe = fe * realDis*1.5 / 907.2973 ;
    edgeLength = fe

    if(color==0):
        discernFlag = 0
    else:
        discernFlag = 2


    print(color,' ',fc,' ',fe,)

    cv.putText(framecopy,str(edgeLength) , (0, 50), cv.FONT_HERSHEY_SIMPLEX, 2, (255, 255, 0), 1, cv.LINE_AA)
    cv.putText(framecopy,str(realDis) , (0, 200), cv.FONT_HERSHEY_SIMPLEX, 1.5, (255, 255, 0), 1, cv.LINE_AA)
    #cv.putText(framecopy, "VolleyBall", (0, 50), cv.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 0), 1, cv.LINE_AA)
    #cv.putText(framecopy, "VolleyBall", (0, 50), cv.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 0), 1, cv.LINE_AA)
    cv.imshow('framecopy',framecopy)

    #error_x,error_y,int(edgeLength),Color,Shape,Ball,discernFlag


    #ret = cv.matchShapes(realTime_cnts,Rec_cnt_ref,1,0.0) #用于匹配形状，可以判断形状了
    #if ret < 0.05 :
    #    print("A Rec")
    #print('ret',ret)
    #print(cnt_tri)
    #print('len=',len(realTime_cnts))
    #TODO:学会contours的扔掉，扔掉面积与一定值的
    #Measure(frame,realTime_red_cnts)
    #Measure(frame,realTime_green_cnts)
    #Measure(frame,realTime_blue_cnts)
    #cv.putText(frame, '1', (int(cx-20), int(cy-20)), cv.FONT_HERSHEY_SIMPLEX,3, (0, 255, 255), 2)
    print('Doing One')

    return color,shapeCode,fe

def DoTaskThree():
    global error_x,error_y,edgeLength,discernFlag
    '''
    color,shapeCode,fe,(err_x,err_y) = DoTaskOne()





    realTime_red_cnts,realTime_green_cnts,realTime_blue_cnts = getContours(frame)

    cx,cy,edgeLength=filterShapes(frame,realTime_green_cnts)

    discernFlag = 1#??Need Change??
    (error_x,error_y) =(cx-320,cy-240)
    print('error = ',error_x,error_y)
    cv.putText(frame, '3', (int(cx-20), int(cy-20)), cv.FONT_HERSHEY_SIMPLEX,3, (0, 255, 255), 2)
    TotalError = abs(error_x)+abs(error_y)
    cv.putText(frame, 'Error ='+ str(TotalError) ,(int(cx-10), int(cy+10)), cv.FONT_HERSHEY_SIMPLEX,2, (0, 0, 255), 2)
    '''
    DoTaskOne()


    '''
    if abs(error_x) <= 5:

        cv.putText(frame, 'Yaw Aim Success' ,(int(cx-10), int(cy-10)), cv.FONT_HERSHEY_SIMPLEX,0.5, (0, 0, 255), 2)

        discernFlag = 2

        if abs(error_y) <= 5:

            discernFlag = 3

        else:
            discernFlag = 2

    else :
        discernFlag = 1
    '''
    #Measure(frame,realTime_green_cnts)
    #Measure(frame,realTime_blue_cnts)

    print('Doing Three')

    return






cap = cv.VideoCapture(0)
frame_rate_calc = 1
freq = cv.getTickFrequency()
font = cv.FONT_HERSHEY_SIMPLEX
Task = 0
realDis =0
error_x =0
error_y = 0
edgeLength = 0
EstimateIndex = 50/122
Shape = 0
Color = 0
Ball = 0
discernFlag = 0
cx= 0
cy = 0
BallType= 0
c = 0
sc = 0
fe = 0
OutOnce = 0
OutOnce3 = 0
while(1):



    ret,raw_frame = cap.read()
    ret,raw_frame = cap.read()

    #raw_frame = cv.undistort(raw_frame,mtx,dist,None,newcameramtx)
    #ret,frame = cap.read()
    '''
    ret = 1

    raw_frame = np.zeros((480,640,3),np.uint8)

    cv.rectangle(raw_frame,(0,0),(640,480),(235,235,235),-1)

    tri = np.array([[270,290],[320,190],[370,290]],np.int32)

    #cv.fillPoly(raw_frame,[tri],(0,0,255))

    cv.rectangle(raw_frame,(270,190),(370,290),(235,2,2),-1)
    '''

    frame = cv.resize(raw_frame,(320,240));

    #frame = frame[20:300,20:220]
    #frame = raw_frame

    t1 = cv.getTickCount()
    #frame = cv.undistort(frame, mtx, dist, None, newcameramtx)
    #cv.imshow('frame',frame)
    if not ret:
        break

    Task,realDis = Usart_Rece()
    #Task= 1
    #realDis = 307

    print('task',Task)
    print('realDis',realDis)



    #time.sleep()
    #print('Dis OK!')
    #if Task == 1 | Task == 2 :
    #c,sc,fe = DoTaskOne()
    #小心 2 不存在 引起的 问题
    '''
    elif Task == 3:
        DoTaskThree()
    '''
    key_num = cv.waitKey(1)
    if key_num == ord('k'):
        BallType = DetectBall(frame)
        Usart_Send(Task,error_x,error_y,int(fe),c,sc,Ball,discernFlag) #此处返回给32

    elif key_num == ord('j'):

        c,sc,fe = DoTaskOne()
        OutOnce = 0
        Usart_Send(Task,error_x,error_y,int(fe),c,sc,Ball,discernFlag) #此处返回给32

    elif key_num == ord('l'):
        DoTaskThree()
        OutOnce3 = 0
        Usart_Send(Task,error_x,error_y,int(fe),c,sc,Ball,discernFlag) #此处返回给32

    if fe != 0 and OutOnce == 0:
        OutOnce = 1


        print('FinalFe' , fe)

    if OutOnce3 == 0:
        OutOnce3 = 1

        #fe = fe * realDis*1.5 / 907.2973 ;

        print('axis' , (error_x,error_y))
    #print('BallType' , BallType)
#fe = fe *




    #可能会写其他的

    #cv.imshow('Ori-edges',edges)
    #cv.drawContours(img, [cnt_tri], 0, (255,255,255), 3)#只绘制最大的边框，但是没法用矩形。(255,255,255)才为白，否则为黑
    #cv.imshow('edges',edges)


    cv.putText(frame, "FPS: {0:.2f}".format(frame_rate_calc), (30, 50), font, 1, (255, 255, 0), 2, cv.LINE_AA)
    cv.imshow('frame',frame)


    t2 = cv.getTickCount()
    time1 = (t2 - t1) / freq
    frame_rate_calc = 1 / time1


    #cv.imshow('frame',frame)
    #cv.drawContours(img, [cnt_tri], 0, (0,255,0), 3)
    #if cv.waitKey(1) & 0xFF == ord('q'):
     #   break

cv.destroyAllWindows()



