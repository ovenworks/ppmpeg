# -*- coding: utf-8 -*-

__author__ = "ppmpeg@ovenworks.jp"

from ppmpeg import _native
from ppmpeg.video_frame import ReadedVideoFrame
from ppmpeg.audio_frame import ReadedAudioFrame

class DtsMediaReader:
    u"""
    DTS メディアリーダークラス
    メディアを読み出し、ビデオフレームとオーディオフレームをデコードするためのオブジェクトです。
    デコードしたフレームは DTS に従った未加工の順番で出力します。
    """
    
    def __init__(self):
        u"""
        コンストラクタ
        """
        _native.MediaReader_init()

    def open(self, filepath):
        u"""
        ファイルをオープンします。
        :param filepath:    ファイルパス
        :return:            self
        """
        _native.MediaReader_open(filepath)
        return self

    def __enter__(self):
        u"""
        with コンテキストを開始します。
        """
        return self
    
    def metadata(self, key, defv):
        u"""
        メタデータから、指定されたキーに対応する値を返します。
        :param key:     キー
        :param defv:    キーが見つからない場合に返す値
        :return:        キーに対応する値
        """
        value = _native.MediaReader_get_metadata(key)
        return defv if value is None else value
    
    @property
    def duration(self):
        u"""
        長さを返します。
        :return:    長さ(s)
        """
        return _native.MediaReader_get_duration()
    
    @property
    def bit_rate(self):
        u"""
        総合ビットレートを返します。
        :return:    ビットレート(bps)
        """
        return _native.MediaReader_get_bit_rate()

    @property
    def has_video_stream(self):
        u"""
        ビデオストリームが存在するかを返します。
        :return:    ビデオストリームが存在するか
        """
        return _native.MediaReader_has_video_stream()
    
    @property
    def video_width(self):
        u"""
        ビデオの幅を返します。
        :return:    ビデオの幅。ビデオストリームが存在しなければ None
        """
        return _native.MediaReader_get_video_width()

    @property
    def video_height(self):
        u"""
        ビデオの高さを返します。
        :return:    ビデオの高さ。ビデオストリームが存在しなければ None
        """
        return _native.MediaReader_get_video_height()

    @property
    def video_pixel_format(self):
        u"""
        ビデオのピクセルフォーマットを返します。
        :return:    ピクセルフォーマット名。ビデオストリームが存在しなければ None
        """
        return _native.MediaReader_get_video_pixel_format()
    
    @property
    def video_frame_rate(self):
        u"""
        ビデオのフレームレートを返します。
        :return:    フレームレート(fps)。ビデオストリームが存在しなければ None
        """
        return _native.MediaReader_get_video_frame_rate()

    @property
    def video_bit_rate(self):
        u"""
        ビデオのビットレート(bps)を返します。
        :return:    ビットレート。ビデオストリームが存在しなければ None
        """
        return _native.MediaReader_get_video_bit_rate()

    @property
    def video_gop_size(self):
        u"""
        ビデオのGOPサイズを返します。
        :return:    GOPサイズ。ビデオストリームが存在しなければ None
        """
        return _native.MediaReader_get_video_gop_size()

    @property
    def video_start_time(self):
        u"""
        ビデオの先頭フレームの時刻を返します。
        :return:    時刻(s)
        """
        return _native.MediaReader_get_video_start_time()

    @property
    def video_codec(self):
        u"""
        ビデオコーデックを返します。
        :return:    ビデオコーデック名。ビデオストリームが存在しなければ None
        """
        return _native.MediaReader_get_video_codec()
    
    @property
    def has_audio_stream(self):
        u"""
        オーディオストリームが存在するかを返します。
        :return:    オーディオストリームが存在するか
        """
        return _native.MediaReader_has_audio_stream()
        
    @property
    def audio_channels(self):
        u"""
        オーディオのチャンネル数を返します。
        :return:    チャンネル数。オーディオストリームが存在しなければ None
        """
        return _native.MediaReader_get_audio_channels()

    @property
    def audio_nb_samples(self):
        u"""
        オーディオのチャンネルあたりのサンプル数を返します。
        :return:    チャンネルあたりのサンプル数。オーディオストリームが存在しなければ None
        """
        return _native.MediaReader_get_audio_nb_samples()

    @property
    def audio_sample_format(self):
        u"""
        オーディオのサンプルフォーマットを返します。
        :return:    サンプルフォーマット名。オーディオストリームが存在しなければ None
        """
        return _native.MediaReader_get_audio_sample_format()

    @property
    def audio_sample_rate(self):
        u"""
        オーディオのサンプルレートを返します。
        :return:    サンプルレート(Hz)。オーディオストリームが存在しなければ None
        """
        return _native.MediaReader_get_audio_sample_rate()

    @property
    def audio_bit_rate(self):
        u"""
        オーディオのビットレートを返します。
        :return:    サンプルレート(bps)。オーディオストリームが存在しなければ None
        """
        return _native.MediaReader_get_audio_bit_rate()

    @property
    def audio_start_time(self):
        u"""
        オーディオの先頭フレームの時刻を返します。
        :return:    時刻(s)
        """
        return _native.MediaReader_get_audio_start_time()

    @property
    def audio_codec(self):
        u"""
        オーディオコーデック名を返します。
        :return:    オーディオコーデック名。オーディオストリームが存在しなければ None
        """
        return _native.MediaReader_get_audio_codec()

    def process_decode(self, share_vpixels=False, share_asamples=False):
        u"""
        デコード処理を進めます。
        :param share_vpixels:   ビデオピクセルデータ領域を他のビデオフレームと共有するか
        :param share_asamples:  オーティサンプルデータ領域を他のオーディオフレームと共有するか
        :return:                VideoFrame ビデオフレームがデコードされた
                                AudioFrame オーディオフレームがデコードされた
                                True デコード処理を継続する
                                False デコード処理が終了した
        """
        result = _native.MediaReader_process_decode(share_vpixels, share_asamples)

        if isinstance(result, bool):
            return result

        if isinstance(result, tuple):
            if result[0] == "V":
                return ReadedVideoFrame(result[1], result[2], result[3], result[4], result[5], share_vpixels, result[6])
            elif result[0] == "A":
                return ReadedAudioFrame(result[1], result[2], result[3], result[4], result[5], share_asamples)

        return False

    @property
    def decoded_video_frames(self):
        u"""
        デコードしたビデオフレーム数を返します。
        """
        return _native.MediaReader_get_decoded_video_frames()
    
    @property
    def decoded_audio_frames(self):
        u"""
        デコードしたオーディオフレーム数を返します。
        """
        return _native.MediaReader_get_decoded_audio_frames()

    def __exit__(self, type, value, traceback):
        u"""
        with コンテキストを終了します。
        """
        self.close()

    def close(self):
        u"""
        クローズします。
        """
        result = _native.MediaReader_close()
        _native.MediaReader_del()
        return result


class MediaReader(DtsMediaReader):
    u"""
    メディアリーダークラス
    メディアを読み出し、ビデオフレームとオーディオフレームをデコードするためのオブジェクトです。
    デコードしたフレームをバッファリングし、PTS に従った順番で出力します。
    """

    def __init__(self):
        u"""
        コンストラクタ
        """
        super().__init__()
        self.__vframe_buf = []
        self.__vframe_q = []

    def process_decode(self, share_vpixels=False, share_asamples=False):
        u"""
        :func: `ppmpeg.DtsMediaReader.process_decode`
        """

        # 出力キューにビデオフレームがあれば先頭のフレームを返す
        if self.__vframe_q:
            return self.__vframe_q.pop(0)

        # デコードする
        frame = super().process_decode(share_vpixels, share_asamples)

        # デコードが完了したらバッファ内容を出力キューにフラッシュする
        if frame is False:
            self.__flush_vframe_buf()
            return True if self.__vframe_q else False

        # ビデオフレームがデコードされたらバッファリングする
        # キーフレームの場合はそれまでのバッファ内容を出力キューにフラッシュする
        elif isinstance(frame, ReadedVideoFrame):

            if frame.key_frame:
                self.__flush_vframe_buf()

            self.__vframe_buf.append(frame)

            return True

        else:
            return frame

    def __flush_vframe_buf(self):
        u"""
        ビデオフレームのバッファ内容を出力キューにフラッシュします。
        """
        self.__vframe_q.extend(self.__vframe_buf)
        self.__vframe_q.sort(key=lambda frame: frame.pts)
        self.__vframe_buf.clear()
        
    @property
    def decoded_video_frames(self):
        u"""
        :func: `ppmpeg.DtsMediaReader.decoded_video_frames`
        """
        return super().decoded_video_frames - len(self.__vframe_q) - len(self.__vframe_buf)
