# 概要

HTTP/2でGETリクエストを送信し、サーバーとのHTTPフレームのやり取りを出力するコマンドラインツールです。

# 何に使うか

curlやブラウザ組込みの開発ツールでは、HTTP/2で通信してもHTTP/1に変換した結果でしか表示されないので、HTTP/2のフレームレベルでの動作を確認したい場合に使用します。

# 動作環境

Linux

(Fedora29で動作確認)

# ビルドに必要なもの

- g++ (c++17に対応したもの)
- boost (*1)
- openssl (*2)

(*1) RedHat系ならboost-devel<br />
(*2) RedHat系ならopenssl-devel

# ビルド方法

トップディレクトリでmake実行

    make

成功すれば、http-dumpというファイルが作成されているはずです。

# 使い方

コマンドラインのオプションは以下のとおりです。

    http-dump <options> <url>
    
    options:
    -v         verbose mode
    --http2    http/2で接続

-vオプションをつけると送受信するHTTPフレームやhttpヘッダーの内容を表示します。-vがない場合はGETリクエストの結果、取得したレスポンスボディを表示します(curlのような動作)。

--http2オプションをつけるとHTTP/2で接続します。サーバー側がHTTP/2に対応していない場合は"http/2 is not selected."のメッセージを表示して終了します。HTTP/2での接続はTLS接続(https)時のみ対応しています。URL指定がhttp://の場合は、本オプションを指定しても無視されます。

--http2オプションをつけない場合は、http/1.1で接続します。

## 使用例

指定URLにHTTP/2で接続し、HTTPフレームの内容とレスポンスボディを表示。

    ./http-dump -v --http2 https://github.co.jp/


指定URLにHTTP/1.1で接続し、HTTPヘッダーとレスポンスボディを表示。"curl -v https://github.co.jp/"と同じような動作になります。

    ./http-dump -v https://github.co.jp/


# 対応していないこと

- リクエスト種別はGETのみ。POST等には対応していません。

- 単純にGETしてレスポンスを受け取るだけなので、使用するストリームは一つだけです。複数ストリームを作成し並行してデータを取得するようなHTTP/2の特徴を活かすような処理はしていません。

- サーバーのSSL証明書の内容はチェックしていません。

- サーバー側からのHEADERSフレームが一つにおさまらないケース(CONTINUATIONフレームが続くケース)

  "CONTINUATION farme not supported"と表示して終了します。

- サーバーから送られてくるSETTINGSフレームは受け取るだけで何もしていません(Windowサイズ設定等)。

- 最低限のGETリクエストを送信して応答を受け取るだけなので、ストリームの状態管理は行っていません。想定していないフレームを受けると、停まったりメッセージを表示して終了します。

- HPACKのエンコードは手を抜いているので、HTTPヘッダーのname,valueとも127文字以下までしか使用できません。指定したURLのパスが長い場合には、"header name/value is too long. Supports less than 127 characters."と表示して終了するかもしれません。

その他、多分いろいろとあるはずです。

# 実行例

コマンドの実行例です。

<pre>
$ ./http-dump -v --http2 https://github.co.jp/

SEND ← 送信フレームの表示
SETTINGS
Header:
00 00 00 04 00 00 00 00 00                        .........

RECV ← 受信フレームの表示
SETTINGS
Header:
00 00 06 04 00 00 00 00 00                        .........
Payload:
00 03 00 00 00 64                                 .....d
SETTINGS_MAX_CONCURRENT_STREAMS: 100

...略...

RECV
HEADERS
Header: (END_HEADERS)
00 01 63 01 04 00 00 00 01                        ..c......
Payload:
88 5f 92 49 7c a5 89 d3 4d 1f 6a 12 71 d8 82 a6   ._.I|...M.j.q...
0b 53 2a cf 7f 76 88 c4 64 e3 b6 35 c8 7a 7f 6c   .S*..v..d..5.z.l
96 d0 7a be 94 13 2a 6e 2d 6a 08 01 7d 40 bf 71   ..z...*n-j..}@.q
91 5c 6d f5 31 68 df 62 8c fe 5b 91 e7 c3 21 63   .\m.1h.b..[...!c

...略...

HEADERSフレーム受信後はHPACKデータを解析してヘッダーリストと構築したHPACK Dynamic Tableの内容を表示。

HPACK Dynamic Table
Table Size: 1123 (Maximum:4096)
[1] x-fastly-request-id 1679b74dcf84bf2c6d9a28563e9590c2ea5b636f
[2] :vary Accept-Encoding
[3] x-timer S1572949792.523242,VS0,VE1

...略...

HPACK Decoded header list
:status 200
:content-type text/html; charset=utf-8
:server GitHub.com
:last-modified Mon, 23 Sep 2019 19:32:59 GMT

...略...

最後にレスポンスヘッダーとレスポンスボディを表示。

</pre>