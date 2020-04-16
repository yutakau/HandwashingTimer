# HandwashingTimer
[English] https://www.github.com/yutakau/HandwashintTimer/README.md

30秒間しっかり手洗いをするためのタイマーです。
水の音を検出して、カウントを開始します。

M5Stick-Cというマイコンモジュールを使用します。このモジュールにはマイクが内蔵されているので、
外付け部品が不要です。

## デモビデオ
Twitter -> https://twitter.com/i/status/1249699639462211584

---
## 部品

 型番    　 |数量
 ----------|---
 M5Stick-C | 1 

## 使用ライブラリ
　ArduinoFFT (https://github.com/kosme/arduinoFFT)
 が必要です。

## しくみ
　水音は、ホワイトノイズに近い特徴的な周波数スペクトルを持ちます。そこで、マイク音をFFTで周波数領域に変換したデータを、
  Support Vetor Machineで比較して、水音かどうかを判定します。
  
## Author
 Yutaka Usui (https://github.com/yutakau)
 
