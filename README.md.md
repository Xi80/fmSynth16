# fmSynth16

STM32とYMF825Boardを使用したスタンドアロンハードウェアMIDI音源。



## ライセンス

fmSynth16はmbedフレームワーク上で動作します、それらの部分はもともとのライセンスに準じます。

fmSynth16のソースコード自体は**[CC BY-NC-ND](https://creativecommons.org/licenses/by-nc-nd/4.0/deed.ja)**です。



## 概要

STM32F303K8T6または同等以上の性能を持ったSTM32 Nucleoボード上で動作するハードウェアMIDI音源です。

音源部にはFM音源であるYMF825を使用しています。

本体にリズム音源以外のGM音色を内蔵しており、PCを介さずスタンドアロンでの使用が可能です。

![Board](IMGS\circuit.png)

## サンプル(演奏風景)

<iframe width="100%" height="300" scrolling="no" frameborder="no" allow="autoplay" src="https://w.soundcloud.com/player/?url=https%3A//api.soundcloud.com/tracks/868361716&color=%23ff5500&auto_play=false&hide_related=false&show_comments=true&show_user=true&show_reposts=false&show_teaser=true&visual=true"></iframe><div style="font-size: 10px; color: #cccccc;line-break: anywhere;word-break: normal;overflow: hidden;white-space: nowrap;text-overflow: ellipsis; font-family: Interstate,Lucida Grande,Lucida Sans Unicode,Lucida Sans,Garuda,Verdana,Tahoma,sans-serif;font-weight: 100;"><a href="https://soundcloud.com/user-273146202" title="Ivory Aveline Haskell" target="_blank" style="color: #cccccc; text-decoration: none;">Ivory Aveline Haskell</a> · <a href="https://soundcloud.com/user-273146202/ethereal-house-mix-with-ymf825-midi-synthesiser" title="鳥の詩 -ethereal house mix- With YMF825 MIDI Synthesiser" target="_blank" style="color: #cccccc; text-decoration: none;">鳥の詩 -ethereal house mix- With YMF825 MIDI Synthesiser</a></div>



## 諸元

> - GM Lite相当(ただし、リズム音源は除く)
> - Sakura303 Ver1.7c時点での情報です
>   - MCU:STM32F303K8T6(72MHz,RAM12KB,Flash64KB)
>   - チャネル数:16
>   - 最大同時発音数:16
>   - 音源IC:YMF825
>   - インプリメンテーションチャートは別途記載
>   - I/F:3.5mmジャック、MIDI IN
>   - 電源:5v 1.0A以上



## 開発環境

> OS : Windows10 x64
>
> PC : HP Z400
>
> IDE: VSCode + PlatformIO



## 回路図

![Schematic](IMGS\hwsch.png)



## 部品

| 名称                               | 型番                                     | 数量 |
| ---------------------------------- | ---------------------------------------- | ---- |
| オーディオ用電解コンデンサ(10u)    | ニチコンMW                               | 2    |
| オーディオ用電解コンデンサ(220u)   | ニチコンMW                               | 1    |
| 積層セラミックコンデンサ(0.1u)     | 指定なし                                 | 5    |
| 2.1mm DCジャック                   | 2DC0005D100(もしくは同等品)              | 1    |
| アルミ電解コンデンサ(330u)         | ルビコンWXA                              | 2    |
| 積層セラミックコンデンサ(0.47u)    | 指定なし                                 | 1    |
| 積層セラミックコンデンサ(4.7u)     | 指定なし                                 | 1    |
| イヤホンジャック                   | MJ-352W-O(もしくは同等品)                | 1    |
| DIN5ピン ソケット                  | KDJ103-5(もしくは同等品)                 | 1    |
| ダイオード                         | 1N4148(もしくは同等品)                   | 1    |
| オペアンプ                         | LME49710(もしくは同等品)                 | 1    |
| 三端子レギュレータ(3.3v 500mA以上) | 指定なし(積セラの定数は必要に応じて変更) | 1    |
| フォトカプラ                       | TLP2630(もしくは同等品)                  | 1    |
| 抵抗(1/4w,47)                      | 指定なし                                 | 1    |
| 抵抗(1/4w,220)                     | 指定なし                                 | 1    |
| 抵抗(1/4w,1k)                      | 指定なし                                 | 1    |
| 抵抗(1/4w,10k)                     | 指定なし                                 | 5    |
| 抵抗(1/4w,47k)                     | 指定なし                                 | 1    |
| ピンソケット(1x6)                  | 指定なし                                 | 1    |
| ピンソケット(1x16)                 | 指定なし                                 | 2    |
| MCU                                | STM32F303K8T6(&DIP変換基板)              | 1    |
| ピンヘッダ(1x16)                   | 指定なし                                 | 1    |
| スイッチ付きボリューム(10kA)       | 寸法が合うものなら指定なし               | 1    |



## ファームウェア書き込み

各自ビルドしてSWD経由で書き込んでください。



## 使い方

1. 電源を接続する
2. MIDIコネクタを接続する

特に音源側でするべき操作はありません。



## MIDIインプリメンテーションチャート

| ファンクション                                               | 送信                                         | 受信                                         | 備考                                                         |
| ------------------------------------------------------------ | -------------------------------------------- | -------------------------------------------- | ------------------------------------------------------------ |
| **ベーシックチャネル**<br>電源ON時<br>設定可能               | <br>X<br>X                                   | <br>1-16<br>1-16                             | 記憶されない                                                 |
| **モード**<br>電源ON時<br>メッセージ<br>代用                 | <br>X<br>\*\*\*\*\*\*                        | <br>3<br>                                    |                                                              |
| **ノートナンバー**<br><br>音域                               | <br>X<br>X                                   | <br>0-127<br>音色依存                        |                                                              |
| **ベロシティ**<br>ノート・オン<br>ノート・オフ<br>           | <br>X<br>X                                   | <br>0-127<br>X                               |                                                              |
| **アフタータッチ**<br>キー別<br>チャネル別                   | <br>X<br>X                                   | <br>X<br>X                                   |                                                              |
| **ピッチ・ベンダー**                                         | X                                            | O                                            | 256段階                                                      |
| **コントロールチェンジ**<br>1<br/>6/38<br/>7<br/>11<br/>64<br/>100<br/>101 | <br><br/>X<br>X<br>X<br>X<br>X<br>X<br>X<br> | <br/><br>O<br>O<br>O<br>O<br>O<br>O<br>O<br> | <br>モジュレーション<br/>データエントリー<br/>メインボリューム<br/>エクスプレッション<br/>ホールド１<br/>RPN LSB<br/>RPN MSB |
| **プログラムチェンジ**<br/><br/>設定可能範囲                 | <br/>X<br/>X                                 | <br/>0-127<br/>0-127                         |                                                              |
| **エクスクルーシブ**                                         | X                                            | O                                            |                                                              |
| **コモン**<br/>ソング・ポジション<br/>ソング・セレクト<br/>チューン | <br/>X<br/>X<br/>X                           | <br/>X<br/>X<br/>X                           |                                                              |
| **リアルタイム**<br/>クロック<br/>コマンド                   | <br/>X<br/>X                                 | <br/>X<br/>X                                 |                                                              |
| **その他**<br/>ローカル ON/OFF<br/>オールノートオフ<br/>アクティブセンシング<br/>リセット | <br/>X<br/>X<br/>X<br/>X                     | <br/>X<br/>O<br/>O<br/>X                     |                                                              |

> 対応SysEx
>
> - GMシステム・オン
> - XGシステム・オン
> - GSリセット



> 対応RPN
>
> - ピッチベンドセンシティビティ(0-24)

## 備考

質問・バグなどは[Twitter](twitter.com/status/LunaTsukinashi)もしくはこの[RepoのIssue](https://github.com/Xi80/fmSynth16/issues)で知らせていただけると幸いです。

コードについては徐々にリファクタリングを進めていきます。

## 参考

- https://qiita.com/Shigosen/items/a087546ba2322b82288f
- https://github.com/yamaha-webmusic/ymf825board
- http://madscient.hatenablog.jp/entry/2017/08/06/173827
- http://madscient.hatenablog.jp/entry/2017/08/06/183052
- http://madscient.hatenablog.jp/entry/2017/08/13/013913

## 改変履歴

> [2020-08-04] - 初版(Ver1.7c)



