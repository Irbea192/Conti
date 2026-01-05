#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SDA_PIN 17
#define SCL_PIN 16

// サーボの横の角度は前を0度、後ろを180度とする
// サーボの縦の角度は上を0度、下を180度とする

// MG90の信号の1周期は50Hz(20ms)
// MG90の最小・最大パルス幅は0.5ms〜2.4ms
// PCA9685ドライバは制御信号を12bitの分解能でPWM出力する
// 0~12bit は0~4095の4096段階
// 0.5msは(0.5/20)*4096=102.4
// 2.4msは(2.4/20)*4096=491.52
#define SERVO_MIN  102
#define SERVO_MAX  492

// 足の番号定義
#define FL 0 // 左前
#define FR 1 // 右前
#define BL 2 // 左後
#define BR 3 // 右後

// PWMドライバのサーボ番号定義
#define FL_V 0 // 左前縦
#define FR_V 2 // 右前縦
#define BL_V 4 // 左後縦
#define BR_V 6 // 右後縦

#define FL_H 1 // 左前横
#define FR_H 3 // 右前横
#define BL_H 5 // 左後横
#define BR_H 7 // 右後横

#define INIT_VANGLE 90 // 初期縦角度
#define INIT_HANGLE 90 // 初期横角度

// 足の情報を格納する構造体
typedef struct {
  const int vertcal_num; // 縦サーボの番号
  const int horizontal_num; // 横サーボの番号
  int v_angle; // 現在の縦角度
  int h_angle; // 現在の横角度
  const bool v_invert; // 縦の反転をするかを示すフラグ
  const bool h_invert; // 横の反転をするかを示すフラグ
} Leg;

// 足の初期化
Leg legs[4] = {
  {FL_V, FL_H, INIT_VANGLE, INIT_HANGLE, false, false}, // 前左
  {FR_V, FR_H, INIT_VANGLE, INIT_HANGLE, false, false}, // 前右
  {BL_V, BL_H, INIT_VANGLE, INIT_HANGLE, false, false}, // 後左
  {BR_V, BR_H, INIT_VANGLE, INIT_HANGLE, false, false}  // 後右 
};

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// サーボを指定した角度に動かす関数
void servo_move(int num, int angle) {
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;
  int pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
  pwm.setPWM(num, 0, pulse);
}

// 足を持ち上げて前後に動かす関数
void leg_upmovedown(Leg *leg, int move_angle, int delay_time) {
  if (leg->h_invert) {
    move_angle = 180 - move_angle; // 反転
  }
  if (leg->h_angle == move_angle) {
    // すでに目的の位置にある場合は何もしない
    return;
  }

  leg->h_angle = move_angle;
  leg->v_angle = 45; // 持ち上げ角度
  if (leg->v_invert) {
    leg->v_angle = 180 - leg->v_angle; // 反転
  }

  servo_move(leg->vertcal_num, leg->v_angle); // 足を持ち上げる
  delay(delay_time);

  servo_move(leg->horizontal_num, leg->h_angle); // 足を前後に動かす
  delay(delay_time);

  leg->v_angle = INIT_VANGLE; // 足を下ろす角度
  servo_move(leg->vertcal_num, leg->v_angle); // 足を下ろす
  delay(delay_time);
}

// 足を前後に動かす関数
void leg_move(Leg *leg, int move_angle) {
  if (leg->h_invert) {
    move_angle = 180 - move_angle; // 反転
  }
  if (leg->h_angle == move_angle) {
    // すでに目的の位置にある場合は何もしない
    return;
  }

  leg->h_angle = move_angle;  
  servo_move(leg->horizontal_num, leg->h_angle); // 足を前後に動かす
}

// 足を初期位置に移動する関数
void legs_init() {
  for (int i = 0; i < 4; i++) {
    servo_move(legs[i].vertcal_num, INIT_VANGLE);
    servo_move(legs[i].horizontal_num, INIT_HANGLE);
    delay(100);
  }
}

void walk_forward() {
  // クリープゲイツで歩行
  // https://makezine.jp/blog/2016/12/robot-quadruped-arduino-program.html

  leg_upmovedown(&legs[FR], 140, 100); // 右前足を140度(-50)

  leg_upmovedown(&legs[BR], 40, 100); // 右後足を40度(50)
  leg_upmovedown(&legs[FR], 80, 100); // 右前足を80度(10)
  
  leg_move(&legs[FR], 90); // 右前足を中央に戻す(0) 
  leg_move(&legs[BR], 90); // 右後足を中央に戻す(0)
  leg_move(&legs[FL], 140); // 左前足を140度(-50)
  leg_move(&legs[BL], 100); // 左後足を100度(-10)

  leg_upmovedown(&legs[BL], 40, 100); // 左後足を40度(50)
  leg_upmovedown(&legs[FL], 80, 100); // 左前足を80度(10)

  leg_move(&legs[FL], 90); // 左前足を中央に戻す(0)
  leg_move(&legs[BL], 90); // 左後足を中央に戻す(0)
  leg_move(&legs[FR], 140); // 右前足を140度(-50)
  leg_move(&legs[BR], 100); // 右後足を100度(-10)
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup start ......");

  Wire.begin(SDA_PIN, SCL_PIN);

  pwm.begin();
  pwm.setPWMFreq(50);

  legs_init();

  Serial.println("Setup done.");
}

void loop() {
  walk_forward();
}
