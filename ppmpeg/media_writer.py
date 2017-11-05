# -*- coding: utf-8 -*-

"""
media_writer.py
"""
__author__ = "ppmpeg@ovenworks.jp"

from . import _native
from .video_frame import VideoFrame
from .audio_frame import AudioFrame

class MediaWriter:
    u"""
    メディアライタークラス
    ビデオデータとオーディオデータをエンコードし、メディアに書き出すためのオブジェクトです。
    """
    
    def __init__(self):
        u"""
        コンストラクタ
        """
        _native.MediaWriter_init()
    
    def open(self, filepath):
        u"""
        ファイルをオープンします。
        :param filepath:    ファイルパス
        :return:            self
        """
        _native.MediaWriter_open(filepath)
        return self

    def __enter__(self):
        u"""
        with コンテキストを開始します。
        """
        return self

    def put_metadata(self, key, value):
        u"""
        メタデータを設定します。
        :param key:     キー
        :param value:   値
        """
        return _native.MediaWriter_put_metadata(key, value)

    def add_video_stream(self, width, height, fps, bit_rate=None, gop_size=None, start_time=None, codec=None):
        u"""
        ビデオストリームを追加します。
        :param width:       幅
        :param height:      高さ
        :param fps:         フレームレート(fps)
        :param bit_rate:    ビットレート(bps)。0ならコーデックのデフォルト値を使用する
        :param gop_size:    GOPサイズ。None ならコーデックのデフォルト値を使用する
        :param start_time:  先頭フレームの開始時刻(s)。None ならデフォルト値を使用する
        :param codec:       コーデック名。None ならデフォルト値を使用する
        :return:            追加できたか。指定された形式がサポートされていない場合は追加できない
        """
        return _native.MediaWriter_add_video_stream(width, height, fps, bit_rate, gop_size, start_time, codec)

    @property
    def has_video_stream(self):
        u"""
        ビデオストリームが存在するかを返します。
        :return:    ビデオストリームが存在するか
        """
        return _native.MediaWriter_has_video_stream()    

    @property
    def video_width(self):
        u"""
        ビデオの幅を返します。
        :return:    ビデオの幅。ビデオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_video_width()

    @property
    def video_height(self):
        u"""
        ビデオの高さを返します。
        :return:    ビデオの高さ。ビデオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_video_height()

    @property
    def video_pixel_format(self):
        u"""
        ビデオのピクセルフォーマットを返します。
        :return:    ピクセルフォーマット名。ビデオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_video_pixel_format()
    
    @property
    def video_frame_rate(self):
        u"""
        ビデオのフレームレートを返します。
        :return:    フレームレート(fps)。ビデオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_video_frame_rate()

    @property
    def video_bit_rate(self):
        u"""
        ビデオのビットレート(bps)を返します。
        :return:    ビットレート。ビデオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_video_bit_rate()

    @property
    def video_gop_size(self):
        u"""
        ビデオのGOPサイズを返します。
        :return:    GOPサイズ。ビデオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_video_gop_size()

    @property
    def video_start_time(self):
        u"""
        ビデオの先頭フレームの時刻を返します。
        :return:    時刻(s)
        """
        return _native.MediaWriter_get_video_start_time()

    @property
    def video_codec(self):
        u"""
        ビデオコーデックを返します。
        :return:    ビデオコーデック名。ビデオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_video_codec()

    def add_audio_stream(self, channels, sample_rate, bit_rate=None, start_time=None, codec=None):
        u"""
        オーディオストリームを追加します。
        :param channels:    チャンネル数
        :param sample_rate: サンプルレート(Hz)
        :param bit_rate:    ビットレート(bps)。None ならコーデックのデフォルト値を使用する
        :param start_time:  先頭フレームの開始時刻(s)。None ならコーデックのデフォルト値を使用する
        :param codec:       コーデック名。None ならデフォルト値を使用する
        :return:            追加できたか。指定された形式がサポートされていない場合は追加できない
        """
        return _native.MediaWriter_add_audio_stream(channels, sample_rate, bit_rate, start_time, codec)

    @property
    def has_audio_stream(self):
        u"""
        オーディオストリームが存在するかを返します。
        :return:    オーディオストリームが存在するか
        """
        return _native.MediaWriter_has_audio_stream()
        
    @property
    def audio_channels(self):
        u"""
        オーディオのチャンネル数を返します。
        :return:    チャンネル数。オーディオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_audio_channels()

    @property
    def audio_nb_samples(self):
        u"""
        オーディオのチャンネルあたりのサンプル数を返します。
        :return:    チャンネルあたりのサンプル数。オーディオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_audio_nb_samples()

    @property
    def audio_sample_format(self):
        u"""
        オーディオのサンプルフォーマットを返します。
        :return:    サンプルフォーマット名。オーディオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_audio_sample_format()

    @property
    def audio_sample_rate(self):
        u"""
        オーディオのサンプルレートを返します。
        :return:    サンプルレート(Hz)。オーディオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_audio_sample_rate()

    @property
    def audio_bit_rate(self):
        u"""
        オーディオのビットレートを返します。
        :return:    ビットレート(bps)。オーディオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_audio_bit_rate()

    @property
    def audio_start_time(self):
        u"""
        オーディオの先頭フレームの時刻を返します。
        :return:    時刻(s)
        """
        return _native.MediaWriter_get_audio_start_time()

    @property
    def audio_codec(self):
        u"""
        オーディオコーデック名を返します。
        :return:    オーディオコーデック名。オーディオストリームが存在しなければ None
        """
        return _native.MediaWriter_get_audio_codec()

    def process_encode(self, frame):
        u"""
        エンコード処理を進めます。
        :param frame:   ビデオフレームかオーディオフレーム
        """
        if isinstance(frame, VideoFrame):
            return _native.MediaWriter_process_encode_video(frame.width,
                                                                   frame.height,
                                                                   frame.pixel_format,
                                                                   frame.pixels)
        elif isinstance(frame, AudioFrame):
            return _native.MediaWriter_process_encode_audio(frame.channels,
                                                                   frame.nb_samples,
                                                                   frame.sample_format,
                                                                   frame.samples)
        else:
            pass #@todo エラーを出すべき？

    @property
    def encoded_video_frames(self):
        u"""
        エンコードしたビデオフレーム数を返します。
        """
        return _native.MediaWriter_get_encoded_video_frames()
    
    @property
    def encoded_audio_frames(self):
        u"""
        エンコードしたオーディオフレーム数を返します。
        """
        return _native.MediaWriter_get_encoded_audio_frames()
        
    def __exit__(self, type, value, traceback):
        u"""
        with コンテキストを終了します。
        """
        self.close()
    
    def close(self):
        u"""
        クローズします。
        """
        _native.MediaWriter_flush_encode()
        result = _native.MediaWriter_close()
        _native.MediaWriter_del()
        return result
