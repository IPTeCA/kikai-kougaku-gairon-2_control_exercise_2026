# -*- coding: utf-8 -*-
"""
Spyderエディタ
"""
from serial import Serial
import re

from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import sys
import math

pi=math.pi

window = None
data =[0]*18
sscale = 1.0
com = 0
xspeed = 0
yspeed = 0
zspeed = 0

def main():
    global com, window
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH)
    glutInitWindowSize(300, 300)     # window size
    glutInitWindowPosition(100, 100) # window position
    glutCreateWindow(b"Paper Plane")      # show window
    display.xrot=display.yrot=display.zrot=0
    glutDisplayFunc(display)         # draw callback function
#    glutDisplayFunc(display)         # draw callback function
    glutReshapeFunc(reshape)         # resize callback function
    glutKeyboardFunc(keyboard)
    glutSpecialFunc(keyboard)
#    glutTimerFunc(scan, 0)
    scan.flg=0
    glutIdleFunc(scan)
    init(300, 300)
    com = Serial(
        port="COM3", baudrate=115200, bytesize=8,
        parity='N', stopbits=1, timeout=None,
        xonxoff=0, rtscts=0, writeTimeout=None, dsrdtr=None)
    glutMainLoop()

def init(width, height):
    """ initialize """
    glClearColor(0.0, 0.0, 0.0, 1.0)
    glEnable(GL_DEPTH_TEST) # enable shading

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    ##set perspective
    gluPerspective(45.0, float(width)/float(height), 0.1, 100.0)

def display():
    """ display """
    global data
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()
    #gluLookAt(0.0, 20.0, 20.0, 0.0, 0.0, 0.0, 0.0, 10.0, -10.0)
    gluLookAt(0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0)
    glRotatef( 90, 1, 0, 0)
    glRotatef(+90, 0, 0, 1)

    glPushMatrix()
    glRotatef(display.xrot, 1, 0, 0)
    glRotatef(display.yrot, 0, 1, 0)
    glRotatef(display.zrot, 0, 0, 1)
    glScalef(sscale,sscale,sscale)
    ##set camera
    ##draw a teapot

########### Yellow #############
    glPushMatrix()
    glRotatef(-float(data[9]), 0, 0, 1)
    glRotatef(-float(data[10]), 0, 1, 0)
    glRotatef(float(data[11]), 1, 0, 0)
    glColor3f(1.0, 1.0, 0.0)
    paperplane()
    glPopMatrix()
    """
########### Green #############
    glPushMatrix()
##    glTranslatef(0, 0, -5)
##    glRotatef(-float(data[9]), 0, 0, 1)
    glRotatef(-float(data[10]), 0, 1, 0)
    glRotatef(float(data[11]), 1, 0, 0)
    glColor3f(0.0, 1.0, 0.0)
    paperplane()
    glPopMatrix()
    """
########### Blue #############
    glPushMatrix()
##    glTranslatef(0, 0, -5)
    glRotatef(math.acos(float(data[12]))/pi*360,  float(data[13]), float(data[14]), float(data[15]))
    #glRotatef(math.acos(float(data[12]))/pi*360,  float(data[13]), -float(data[14]), -float(data[15]))
    glColor3f(0.0, 0.0, 1.0)
    paperplane()
    glColor3f(1.0, 0.0, 1.0)
    glBegin(GL_LINES);
    glVertex3f( 0, 0, 0 )
    glVertex3f( 0.01*float(data[0]), 0.01*float(data[1]), 0.01*float(data[2]) )
    glEnd()
    glPopMatrix()

    glPushMatrix()
    glRotatef(-math.acos(float(data[12]))/pi*360,  float(data[13]), -float(data[14]), -float(data[15]))
    glBegin(GL_LINES);
    glColor3f(1.0, 1.0, 1.0)
    glVertex3f( 0, 0, 0 )
    glVertex3f( 0.01*float(data[0]), 0.01*float(data[1]), 0.01*float(data[2]) )
    glEnd()

    glPopMatrix()

    glPopMatrix()
    glFlush()  # enforce OpenGL command
    display.xrot += xspeed
    display.yrot += yspeed
    display.zrot += zspeed


def paperplane():
    glTranslate(7, 0, 0)
    glBegin(GL_TRIANGLE_FAN)
#    glBegin(GL_TRIANGLE)
    glVertex3d(0, 0, 0)
    glVertex3d(-10, 3, -1)
    glVertex3d(-10, 0, 0)
    glVertex3d(-10, 0, 1)
    glVertex3d(-10, 0, 0)
    glVertex3d(-10, -3, -1)
    glEnd()
    glTranslate(-7, 0,  0)
    glutSwapBuffers()
               
def paperplane00():
    glTranslate(0, 0, -3)
    glBegin(GL_TRIANGLE_FAN)
#    glBegin(GL_TRIANGLE)
    glVertex3d(0, 0, 0)
    glVertex3d(-3, 1, 5)
    glVertex3d(0, 0, 5)
    glVertex3d(0, -1, 5)
    glVertex3d(0, 0, 5)
    glVertex3d(3, 1, 5)
    glEnd()
    glTranslate(0, 0,  3)
    glutSwapBuffers()
               
def paperplane1():
    glTranslate(0, 0, -20)
    glBegin(GL_TRIANGLE_FAN)
#    glBegin(GL_TRIANGLE)
    glVertex3d(0, 0, 0)
    glVertex3d(-30, 5, -50)
    glVertex3d(0, 0, -50)
    glVertex3d(0, -5, -50)
    glVertex3d(0, 0, -50)
    glVertex3d(30, 5, -50)
    glEnd()
    glTranslate(0, 0,  20)
    glutSwapBuffers()
               
def paperplane0():
    glBegin(GL_TRIANGLE_FAN)
#    glBegin(GL_TRIANGLE)
    glVertex3d(0, 0, 0)
    glVertex3d(-30, 5, -50)
    glVertex3d(-5, 0, -50)
    glVertex3d(0, 20, -50)
    glVertex3d(5, 0, -50)
    glVertex3d(30, 5, -50)
    glEnd()
    glutSwapBuffers()
               
def reshape(width, height):
    """callback function resize window"""
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45.0, float(width)/float(height), 0.1, 100.0)

def scan():
    global com, data
#    scan.flg=0

#    while 1:
    #r_data = com.read_until(size=250)  #size分Read
    #r_data = com.read_until(size=250).decode().replace('\r', '').replace('\n', '').replace(',', ' ') #size分Read
    r_data = com.read_until(size=250).decode()
    #r_data = com.read_until(size=250).decode().replace('\r', '').replace('\n', '').partition(',')  #size分Read
    print(r_data)
    #print(type(r_data))
    #print(len(r_data))
    #data[0]=float(r_data[0])
    #data[1]=r_data.partition(',')
    #data[2]=float(data[2].rpartition(','))
    #data= r_data.partition(',')
    data1 = re.split(',|,|,|,|,', r_data)
    #data1 = re.split('a|a', r_data)
    #data= r_data.split(",")
    print(data1[0])       
    data[9] = float(data1[2])
    data[10] = float(data1[1])
    data[11] = float(data1[0])
    data[0] = float(data1[3])
    data[1] = float(data1[4])
    data[2] = float(data1[5])
    #print(data[0], data[1], data[2])
    """
    if scan.flg :
        data= re.split(",", r_data.decode())
        print(data[0], data[1], data[2])        
#        print(data[9], data[10], data[11], data[12])        
#        print(float(data[12]), data[13], data[14], data[15])        
    else:
        l = len(r_data)
        if l > 100: scan.flg = 1
        print("len= %d, flg=%d" % (l, scan.flg))
#        display()
    """
    glutPostRedisplay()
    
    yaw = math.radians(data[9])
    pitch = math.radians(data[10])
    roll = math.radians(data[11])
    q = euler_to_quaternion(roll, -pitch, -yaw)
    data[12] = q[0]
    data[13] = q[1]
    data[14] = q[2]
    data[15] = q[3]
    

def keyboard(key, x, y):
    global sscale, xspeed, yspeed, zspeed, com, window
    
    if key == b'\x1b':  # ESC
        com.close()
        glutDestroyWindow(window)
        sys.exit()

    #回転速度
    if key == GLUT_KEY_UP:
        xspeed -= 0.1
    elif key == GLUT_KEY_DOWN:
        xspeed += 0.1
    elif key == GLUT_KEY_RIGHT:
        yspeed += 0.1
    elif key == GLUT_KEY_LEFT:
        yspeed -= 0.1
    elif key == b'Z':
        zspeed += 0.1
    elif key == b'z':
        zspeed -= 0.1
    elif key == b'L':
        sscale += 0.1
    elif key == b'S':
        sscale -= 0.1
    print(key, xspeed, yspeed, sscale)

def euler_to_quaternion(roll, pitch, yaw):
    """
    Roll, pitch, and yaw angles (in radians) to quaternion conversion.
    """
    cy = math.cos(yaw * 0.5)
    sy = math.sin(yaw * 0.5)
    cp = math.cos(pitch * 0.5)
    sp = math.sin(pitch * 0.5)
    cr = math.cos(roll * 0.5)
    sr = math.sin(roll * 0.5)

    q_w = cr * cp * cy + sr * sp * sy
    q_x = sr * cp * cy - cr * sp * sy
    q_y = cr * sp * cy + sr * cp * sy
    q_z = cr * cp * sy - sr * sp * cy

    return q_w, q_x, q_y, q_z

#print("Quaternion: q_w = {:.4f}, q_x = {:.4f}, q_y = {:.4f}, q_z = {:.4f}".format(*quaternion))

    
if __name__ == "__main__":
    main()