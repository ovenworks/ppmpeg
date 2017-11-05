# Python-FFmpeg 動画処理ライブラリ

## １．概要

Python-FFmpeg 動画処理ライブラリ（ppmpeg）は Python 上で FFmpeg を用いた動画処理を行うライブラリです。
大きく分類して以下のような機能を提供します。

* FFmpeg 自体の情報取得機能
    * バージョン情報
    * コーデック一覧
    * ビデオピクセル・オーディオサンプルフォーマット一覧など
* メディア読み込み機能
    * メタ情報の取得
    * メディアプロパティの取得（長さ、再生開始時刻など）
    * ビデオ・オーディオストリームの取得
    * ビデオ・オーディオフレームのデコード
* メディア書き出し機能
    * メタ情報の設定
    * ビデオ・オーディオストリームの追加
    * ビデオ・オーディオフレームのエンコード
* ユーティリティ機能
    * ビデオピクセルフォーマットの変換機能
    * ビデオピクセルを画像ファイルとして簡易的に出力する機能
    * オーディオサンプルフォーマットの変換機能
    * オーディオサンプルを音声ファイルとして簡易的に出力する機能

本ライブラリは Python によるクラスライブラリとその内部から呼び出される C 拡張モジュールから成っています。
画像処理を行う PIL や numpy といったライブラリと同様の構成です。


## ２．ファイル構成

* ppmpeg/
ライブラリ本体を格納しています。

* test/
簡易的なテストを格納しています。

* test_resource/
テストで使用するリソースを格納しています。

* docs/
API ドキュメントを生成するための sphinx プロジェクトを格納しています。

* example/
サンプルプログラムを格納しています。

* setup.py
セットアップスクリプトです。

* requirements.txt
依存パッケージとそのバージョンを記述したファイルです。


## ３．開発および動作環境

***MacOS***

* OS : macOS Sierra
* シェル : bash 4.4.12

***Linux***

* OS : CentOS 6.9
* シェル : bash 4.1.2

***AWS***

* AMI : amzn-ami-hvm-2016.03.3.x86_64-gp2
* シェル : bash 4.2.46

仮想環境や、MSYS2/MinGW64 を用いて Windows 上で Linux をシミュレートした環境でも動作します。


## ４．事前準備

* gcc や pkg-config といった C 言語開発に必要なパッケージをインストールしておいてください。
ライブラリのビルド中にコマンドが見つからないというエラーが出た場合は都度インストールしてください。

* Python 3.6.1（またはそれと互換性のあるバージョン）のコマンドとヘッダ・ライブラリ一式をインストールしておいてください。
OS 標準のパッケージマネージャーでインストールできない場合はソースからビルド・インストールしてください。  
また pip コマンドも使用できるようにしておいてください。

* FFmpeg 3.3.3（またはそれと互換性のあるバージョン）のコマンドとヘッダ・ライブラリ一式をインストールしておいてください。
OS 標準のパッケージマネージャーでインストールできない場合はソースからビルド・インストールしてください。

* Python と FFmpeg のヘッダ・ライブラリ格納ディレクトリを PKG_CONFIG_PATH に適切に設定しておいてください。
また共有ライブラリの格納ディレクトリを LD_LIBRARY_PATH に設定しておいてください。

```
# Python と FFmpeg を実行できるか確認
$ echo $PATH
$ python3 --version
$ pip3 --version
$ ffmpeg --version

# PKG_CONFIG_PATH に Python と FFmpeg のヘッダとライブラリが登録されているか確認
$ echo $PKG_CONFIG_PATH
$ pkg-config --cflags --libs python-3.6
$ pkg-config --cflags --libs libavformat
$ pkg-config --cflags --libs libavcodec
$ pkg-config --cflags --libs libavdevice

# LD_LIBRARY_PATH に FFmpeg と Python の共有ライブラリが登録されているか確認
$ echo $LD_LIBRARY_PATH
$
$ cd python3 コマンドがあるディレクトリ
$ ldd python3    # 全ての共有ライブラリが解決されていればOK
$
$ cd ffmpeg コマンドがあるディレクトリ
$ ldd ffmpeg     # 全ての共有ライブラリが解決されていればOK
...
```

## ５．動作テスト

セットアップスクリプトを用いてビルドとテストを実行します。
テストがエラーなく完了し、test_resource フォルダ内に画像ファイルや動画ファイルができていれば成功です。

```
$ python3 setup.py check build
$ python3 setup.py test
```

## ６．インストール

セットアップスクリプトを用いてインストールします。

```
$ python3 setup.py install
$ pip3 list | grep ppmpeg
```

これで Python スクリプトから ppmpeg パッケージをインポートして使用できるようになります。


## ７．APIドキュメントの生成

docs ディレクトリに sphinx ドキュメントジェネレータ向けのプロジェクトを用意しています。
sphinx をインストールして API ドキュメントを生成してください。
例えば以下のようにすると docs/_build/html ディレクトリ内に HTML 形式の API ドキュメントが生成されます。

```
$ pip3 install sphinx
$ pip3 list | grep sphinx
$ cd docs
$ make html
```


## ８．サンプルプログラム

example ディレクトリに、本ライブラリを用いた処理のサンプルプログラムを格納しています。
また test ディレクトリ中にある簡易テストもミニマムなサンプルとして参照してください。


## 

