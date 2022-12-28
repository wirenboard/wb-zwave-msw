# Получение данных об устройстве

1. Для работы необходимо скачать и распаковать утилиту [zme_make](https://github.com/Z-Wave-Me/Z-Uno-G2-Core/tree/master/toolchain/linux64), подключить плату.
2. Код DSK можно получить в выводе команды получения информации об устройстве в разделе **SECURITY**
```sh
./zme_make boardInfo -d /dev/ttyUSB0 
```

3. Код QR в текстовом виде также можно получить в разделе **SECURITY**, а в виде png-файла при помощи команды 
```sh
 ./zme_make boardInfo -d /dev/ttyUSB0 -q qr.png
 ```