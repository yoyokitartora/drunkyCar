# drunkyCar 多快好省


## 核心思想:

1、CNN太慢，机器视觉也可以搞定道路分割，只不过太烧脑

2、智商=300-源文件行数。

## 如何使用：

般情况下只有一个App.cpp,不定期release。

仅依赖：OpenCV 3.x 带contrib

## 更新日志：

### 6-1 

1. 首次公开代码，效果见视频

### 6-4 

1. 使用上一帧路径点位置信息来搜索，更稳定了;

2. 适用于木地板，瓷砖，柏油路等各种平面;

3. Pipeline性能优化，处理时间属于10ms级别（i5-5300u单核@windows10）;

4. 写了一个绘图函数用于装逼:
<div align=center>
![](https://raw.githubusercontent.com/BICHENG/drunkyCar/master/demo_media/6-4-v0.1/roadseg1.png)

![](https://raw.githubusercontent.com/BICHENG/drunkyCar/master/demo_media/6-4-v0.1/roadseg2.png)

![](https://raw.githubusercontent.com/BICHENG/drunkyCar/master/demo_media/6-4-v0.1/roadseg3.png)

![](https://raw.githubusercontent.com/BICHENG/drunkyCar/master/demo_media/6-4-v0.1/roadseg4.png)

![](https://raw.githubusercontent.com/BICHENG/drunkyCar/master/demo_media/6-4-v0.1/roadseg5.png)
</div>

## 参考

Structured Forests for Fast Edge Detection

https://www.microsoft.com/en-us/research/publication/structured-forests-for-fast-edge-detection/

model文件：

https://github.com/opencv/opencv_extra/blob/master/testdata/cv/ximgproc/model.yml.gz


## 视频：

https://youtu.be/-cr9FKWecgE

https://youtu.be/2d_pJL0T6U0

https://www.bilibili.com/video/av24232013/
