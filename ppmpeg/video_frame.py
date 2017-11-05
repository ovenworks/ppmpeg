# -*- coding: utf-8 -*-

"""
video_frame.py
"""
__author__ = "ppmpeg@ovenworks.jp"

from .media_util import MediaUtil

class VideoFrame:
    u"""
    ビデオフレームクラス
    ビデオフレームの情報を保持するオブジェクトです。
    """
    
    def __init__(self, pts, width, height, pixel_format, pixels):
        u"""
        コンストラクタ
        :param pts:             PTS
        :param width:           幅
        :param height:          高さ
        :param pixel_format:    ピクセルフォーマット名
        :param pixels:          ピクセルデータ(bytes)
        """
        self.__pts = pts
        self.__width = width
        self.__height = height
        self.__pixel_format = pixel_format
        self.__pixels = pixels

    @property
    def pts(self):
        u"""
        PTSを返します。
        :return:    PTS
        """
        return self.__pts

    @property
    def width(self):
        u"""
        幅を返します。
        :return:    幅
        """
        return self.__width
    
    @property
    def height(self):
        u"""
        高さを返します。
        :return:    高さ
        """
        return self.__height

    @property
    def pixel_format(self):
        u"""
        ピクセルフォーマットを返します。
        :return:    ピクセルフォーマット名
        """
        return self.__pixel_format

    @property
    def pixels(self):
        u"""
        ピクセルデータを返します。
        :return:    ピクセルデータ(bytes)
        """        
        return self.__pixels

    def converted(self, width, height, pixel_format):
        u"""
        指定されたサイズとピクセルフォーマットに変換したビデオフレームを作成して返します。
        :param width:   幅
        :param height:  高さ
        :pixel_format:  ピクセルフォーマット名
        :return:        変換したビデオフレーム
        """
        pixels = MediaUtil.convert_pixels(self.__width,
                                          self.__height,
                                          self.__pixel_format,
                                          self.__pixels,
                                          width,
                                          height,
                                          pixel_format)

        return VideoFrame(self.__pts, width, height, pixel_format, pixels)

class ReadedVideoFrame(VideoFrame):
    u"""
    読み出し済みビデオフレームクラス
    読み出したビデオフレームの情報を保持するオブジェクトです。
    """
    
    def __init__(self, pts, width, height, pixel_format, pixels, pixels_shared, key_frame):
        u"""
        コンストラクタ
        :see:                   VideoFrame#__init__()
        :param pixels_shared:   ピクセルデータ領域を他のフレームと共有しているか
        :param key_frame:       キーフレームか
        """
        super().__init__(pts, width, height, pixel_format, pixels)
        self.__pixels_shared = pixels_shared
        self.__key_frame = key_frame

    @property
    def pixels_shared(self):
        u"""
        ピクセルデータ領域を他のフレームと共有しているかを返します。
        :return:    共有しているか
        """
        return self.__pixels_shared

    @property
    def key_frame(self):
        u"""
        キーフレームかを返します。
        :return:    キーフレームか
        """
        return self.__key_frame