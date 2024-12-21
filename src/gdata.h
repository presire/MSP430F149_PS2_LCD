#ifndef GDATA_H
#define GDATA_H

#define BufferSize  32                      // 表示バッファサイズ
extern unsigned char bitcount;              // ビットカウンタ
extern unsigned char kb_buffer[BufferSize]; // 表示バッファ
extern unsigned char input;                 // データ書き込み位置
extern unsigned char output;                // データ読み出し位置
extern unsigned char pebit;                 //パリティビット
extern unsigned char recdata;               // 受信データ
extern unsigned char tishi[];               // 表示用文字列

#endif
