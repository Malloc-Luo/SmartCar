import cv2
import numpy as np
from keras.models import load_model
from training import Training
import os
from keras import backend
import time
import random
from scipy import stats
import serial
import threading
from red import detect_red_car

uart = serial.Serial("/dev/ttyUSB0", 115200)

mid_pre=[]
cnt = 0
final_prediction = 6
thresh = np.zeros((3,3), np.uint8)
def undistort(frame):
    fx = 1161.9
    cx = 303.2
    fy = 1159.2
    cy = 265.1
    k1, k2, p1, p2, k3 = -0.5372, 3.6485, 0.0, 0.0, 0.0
    k = np.array([
        [fx, 0, cx],
        [0, fy, cy],
        [0, 0, 1]
    ])
    d = np.array([
        k1, k2, p1, p2, k3
    ])
    h, w = frame.shape[:2]
    mapx, mapy = cv2.initUndistortRectifyMap(k, d, None, k, (w, h), 5)
    return cv2.remap(frame, mapx, mapy, cv2.INTER_LINEAR)

def send_uart():
    global final_prediction, thresh
    while True:
        time.sleep(1.5)
        if thresh is None:
            uart.write(b'0')
        elif final_prediction == 0:
            uart.write(b'1')
        elif final_prediction == 2:
            uart.write(b'2')
        else:
            uart.write(b'3')
        print('send ', final_prediction)


class Gesture():
    def __init__(self, train_path, predict_path, gesture, train_model):
        self.blurValue = 5
        self.bgSubThreshold = 36
        self.train_path = train_path
        self.predict_path = predict_path
        self.threshold = 60
        self.gesture = gesture
        self.train_model = train_model
        self.skinkernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5, 5))
        self.x1 = 380
        self.y1 = 60
        self.x2 = 640
        self.y2 = 350

    def collect_gesture(self, capture, ges, photo_num):
        photo_num = photo_num
        vedeo = False
        predict = False
        count = 0
        # 读取默认摄像头
        cap = cv2.VideoCapture(capture)
        # 设置捕捉模式
        cap.set(10, 200)
        cap.set(cv2.CAP_PROP_BRIGHTNESS, 1)
        cap.set(cv2.CAP_PROP_CONTRAST,60) # 对比度 40
        cap.set(cv2.CAP_PROP_SATURATION, 40) # 饱和度 50
        cap.set(cv2.CAP_PROP_HUE, 50) # 色调 50
        cap.set(cv2.CAP_PROP_EXPOSURE, -5) # 曝光 50
        # 背景减法创建及初始化
        bgModel = cv2.createBackgroundSubtractorMOG2(0, self.bgSubThreshold)

        while True:
            global cnt, final_prediction, thresh

            cnt += 1
            # 读取视频帧
            ret, frame = cap.read()
            # 镜像转换
            frame = cv2.flip(frame, 1)
             # 双边滤波
            frame = cv2.bilateralFilter(frame, 5, 50,100)
            thresh = detect_red_car(frame)

            if cnt == 5:
                predict = True
                while True:
                    model_name = 'Gesture_2.h5'
                    if model_name == 'exit':
                        break
                    if model_name in os.listdir('./'):
                        print('正在加载{}模型'.format(model_name))
                        p_model = load_model(model_name)
                        break
            if cnt > 6:
                cnt = 6
            if thresh is not None:
                Ges = cv2.resize(thresh, (100, 100))
                if predict == True:
                    img = np.array(Ges).reshape(-1, 100, 100, 1)/255
                    prediction = p_model.predict(img)
                    mid_prediction = [result.argmax() for result in prediction][0]
                    mid_pre.append(mid_prediction)

                    if len(mid_pre) == 3:
                        final_prediction = stats.mode(mid_pre)[0][0]
                        del mid_pre[:]

                        ges_type = self.gesture[final_prediction]
                        #print(ges_type, np.max(prediction[0]))
                        print(ges_type)

                if vedeo is True and count < photo_num:
                    # 录制训练集
                    cv2.imencode('.jpg', Ges)[1].tofile(self.train_path + '{}_{}.jpg'.format(str(random.randrange(1000, 100000)),str(ges)))
                    count += 1
                    print(count)
                elif count == photo_num:
                    print('{}张测试集手势录制完毕，3秒后录制此手势测试集，共{}张'.format(photo_num, int(photo_num*0.43)))
                    time.sleep(3)
                    count += 1
                elif vedeo is True and photo_num < count < int(photo_num*1.43):
                    cv2.imencode('.jpg', Ges)[1].tofile(self.predict_path + '{}_{}.jpg'.format(str(random.randrange(1000, 100000)),str(ges)))
                    count += 1
                    print(count)
                elif vedeo is True and count >= int(photo_num*1.43):
                    vedeo = False
                    ges += 1
                    if ges < len(self.gesture):
                        print('此手势录制完成，按l录制下一个手势')
                    else:
                        print('手势录制结束, 按t进行训练')

                #k = cv2.waitKey(10)
                # if k == 27:
                #     break

                # elif k == ord('l'):  # 录制手势
                #     vedeo = True
                #     count = 0

                # elif k == ord('p'):  # 预测手势
                #     predict = True
                #     while True:
                #         model_name = 'Gesture_2.h5'
                #         if model_name == 'exit':
                #             break
                #         if model_name in os.listdir('./'):
                #             print('正在加载{}模型'.format(model_name))
                #             p_model = load_model(model_name)
                #             break
                #         else:
                #             print('模型名字输入错误，请重新输入，或输入exit退出')

                # elif k == ord('b'):
                #     bgModel = cv2.createBackgroundSubtractorMOG2(0, self.bgSubThreshold)
                #     print('背景重置完成')

                # elif k == ord('t'):
                #     os.environ["CUDA_VISIBLE_DEVICES"] = "0"
                #     train = Training(batch_size=32, epochs=5, categories=len(self.gesture), train_folder=self.train_path,
                #                      test_folder=self.predict_path, model_name=self.train_model, type=self.gesture)
                #     train.train()
                #     backend.clear_session()
                #     print(f'{self.train_model}模型训练结束')



if __name__ == '__main__':

    # 要训练的手势类型
    # Gesturetype = input('请输入训练手势(用逗号隔开)：\n')
    # if Gesturetype == "none":
    #     Gesturetype = ['666', 'yech', 'stop', 'punch', 'OK']
    # else:
    #     Gesturetype = Gesturetype.split(',')

    Gesturetype = ['666', 'yech', 'stop', 'punch', 'OK']
    train_path = 'Gesture_train/'
    pridect_path = 'Gesture_predict/'

    # # 训练集路径
    # train_path = 'train_test/'
    # # 测试集路径
    # pridect_path = 'predict_test/'

    for path in [train_path, pridect_path]:
        if not os.path.exists(path):
            os.mkdir(path)
    print(f'训练手势有：{Gesturetype}')

    # 模型保存命名
    # train_model = input('请输入训练模型名：\n')

    train_model = 'Gesture.h5'
    # 初始化手势识别类
    Ges = Gesture(train_path, pridect_path, Gesturetype, train_model)
    # 单个手势要录制的数量
    num = 500
    # 训练手势类别计数器
    x = 0
    t = threading.Thread(target=send_uart)
    t.start()

    # 调用启动函数
    Ges.collect_gesture(capture=0, ges=x, photo_num=num)
    t.join()