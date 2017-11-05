# -*- coding: utf-8 -*-

__author__ = "ppmpeg@ovenworks.jp"

from ppmpeg import _native

class FFmpeg:
    u"""
    FFmpeg クラス
    バインディングされている FFmpeg の情報を保持しているオブジェクトです。
    """
        
    def __init__(self):
        u"""
        コンストラクタ
        """
        _native.FFmpeg_init()
    
    @property
    def version(self):
        u"""
        バージョン番号を返します。
        :return:    バージョン番号
        """
        return _native.FFmpeg_get_version()
    
    @property
    def configuration(self):
        u"""
        FFmpeg をビルドした際のオプション等の構成情報を返します。
        :return:    構成情報文字列
        """
        return _native.FFmpeg_get_configuration()
    
    @property
    def video_codecs(self):
        u"""
        サポートしているビデオコーデックの一覧を返します。
        :return:    ビデオコーデック名の配列
        """
        return _native.FFmpeg_get_video_codecs()

    def video_pixel_formats(self, vcodec):
        u"""
        指定されたビデオコーデックがサポートしているピクセルフォーマットの一覧を返します。
        :param vcodec:  ビデオコーデック名
        :return:        ピクセルフォーマット名の配列。指定されたビデオコーデックがサポートされていなければ None
        """
        return _native.FFmpeg_get_video_pixel_formats(vcodec)
    
    @property
    def audio_codecs(self):
        u"""
        サポートしているオーディオコーデックの一覧を返します。
        :return:    オーディオコーデック名の配列
        """
        return _native.FFmpeg_get_audio_codecs()
    
    def audio_sample_formats(self, acodec):
        u"""
        指定されたオーディオコーデックがサポートしているサンプルフォーマットの一覧を返します。
        :param acodec:  オーディオコーデック名
        :return:        オーディオサンプルフォーマット名の配列。指定されたオーディオコーデックがサポートされていなければ None
        """
        return _native.FFmpeg_get_audio_sample_formats(acodec)


