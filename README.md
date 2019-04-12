# M5StickC-IR
=====
## これは何

M5StickCに内蔵された赤外線LEDを使用して家電のリモコンとして使えるようにします。


## 使い方

初めて電源を入れるとアクセスポイントモードで起動するので、スマホなどで「 M5StickC-WiFi 」に接続。

ウェブブラウザで「 192.168.4.1 」を開き、使用しているWiFiルーター/アクセスポイントなどのSSIDとパスワードを入力してOk。

再起動すると、指定のWiFiに接続される。

M5StickC本体のLCDに表示されるIPアドレスにウェブブラウザでアクセス。データ送信、本体LEDのON/OFFなどが行える。

送信データはIRKkit互換。

送信データの作成はM5StickC本体だけだと無理なので、各社TVを操作できるウェブアプリを作りました。

http://m5.linclip.com/ir/

## 参考にしたもの

* [minlRum](https://github.com/9SQ/minIRum)

* [IRKit](http://getirkit.com/)

特に、データ送信に関してはminlRumのコードをそのまま使用させていただきました。

## 参考にしたもの

*https://github.com/Brunez3BD/WIFIMANAGER-ESP32

*https://github.com/interactive-matter/aJson

*https://github.com/SensorsIot/Arduino-IRremote

## Author

linclip

