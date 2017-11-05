# -*- coding: utf-8 -*-

__author__ = "ppmpeg@ovenworks.jp"

from ppmpeg import _native

class MediaUtil:
    u"""
    メディアユーティリティクラス
    メディア操作を行う際によく使う機能を定義しているクラスです
    """

    __inited = False

    @classmethod
    def __init_native(cls):
        u"""
        ネイティブモジュール内のインスタンスを初期化します。
        """
        if not cls.__inited:
            _native.MediaUtil_init()
            cls.__inited = True
    
    @classmethod
    def time_to_video_frame(cls, time, fps):
        u"""
        時刻からビデオフレーム位置へ変換します。
        :param time:    時刻(s)
        :param fps:     FPS
        :return:        ビデオフレーム位置
        """
        pass
    
    @classmethod
    def video_frame_to_time(cls, frame, fps):
        u"""
        ビデオフレーム位置から時刻へ変換します。
        :param frame:   ビデオフレーム位置
        :param fps:     FPS    
        :return:        時刻(s)
        """
        pass
    
    @classmethod
    def time_to_audio_frame(cls, time, srate):
        u"""
        時刻からオーディオフレーム位置へ変換します。
        :param time:    時刻(s)
        :param srate:   サンプリングレート(Hz)
        :return:        オーディオフレーム位置
        """
        pass
    
    @classmethod
    def audio_frame_to_time(cls, frame, srate):
        u"""
        オーディオフレーム位置から時刻へ変換します。
        :param frame:   オーディオフレーム位置
        :param srate:   サンプリングレート(Hz)
        :return:        時刻(s)
        """
        pass

    @classmethod
    def resample_video_frames(cls, src_frames, src_frame_rate, dst_frame_rate):
        u"""
        ビデオフレームをリサンプリングします。
        :param src_frames:      リサンプリング元のビデオフレームリスト
        :param src_frame_rate:  リサンプリング元のフレームレート(fps)
        :param dst_frame_rate:  リサンプリング先のフレームレート(fps)
        """
        if not src_frames:
            return src_frames
        elif src_frame_rate == dst_frame_rate:
            return src_frames
        elif src_frame_rate < dst_frame_rate:
            raise NotImplementedError("Upsample video frame is not implement yet.")

        dst_frames = []
        last_frame_no = -1

        for src_frame in src_frames:

            frame_no = int((src_frame.pts - src_frames[0].pts) * dst_frame_rate)

            if frame_no != last_frame_no:
                dst_frames.append(src_frame)
                last_frame_no = frame_no

        return dst_frames

    @classmethod
    def resample_audio_frames(cls, src_frames, src_sample_rate, dst_sample_rate):
        u"""
        オーディオフレームをリサンプリングします。
        :param src_frames:      リサンプリング元のオーディオフレームリスト
        :param src_sample_rate: リサンプリング元のサンプルレート(Hz)
        :param dst_sample_rate: リサンプリング先のサンプルレート(Hz)
        """
        if not src_frames:
            return src_frames
        elif src_sample_rate == dst_sample_rate:
            return src_frames

        raise NotImplementedError("Resample audio frames is not implement yet.")

    @classmethod
    def convert_pixels(cls,
                       src_width, src_height, src_format, src_pixels,
                       dst_width, dst_height, dst_format):
        u"""
        指定されたサイズとフォーマットに変換したピクセルデータを返します。
        :param src_width:   変換元の幅
        :param src_height:  変換元の高さ
        :param src_format:  変換元のピクセルフォーマット名
        :param src_pixels:  変換元のピクセルデータ
        :param dst_width:   変換先の幅
        :param dst_height:  変換先の高さ
        :param dst_format:  変換先のピクセルフォーマット名
        :return:            変換したピクセルデータ
        """
        cls.__init_native()
        return _native.MediaUtil_convert_pixels(src_width,
                                                       src_height,
                                                       src_format,
                                                       src_pixels,
                                                       dst_width,
                                                       dst_height,
                                                       dst_format)

    @classmethod
    def convert_samples(cls,
                        src_channels, src_nb_samples, src_format, src_samples,
                        dst_channels, dst_nb_samples, dst_format):
        u"""
        指定されたチャンネル数、サンプル数とフォーマットに変換したサンプルデータを返します。
        :param src_channels:    変換元のチャンネル数
        :param src_nb_samples:  変換元のチャンネルあたりのサンプル数
        :param src_format:      変換元のサンプルフォーマット名
        :param src_samples:     変換元のサンプルデータ
        :param dst_channels:    変換先のチャンネル数
        :param dst_nb_samples:  変換先のチャンネルあたりのサンプル数
        :param dst_format:      変換先のサンプルフォーマット名
        :return:                変換したサンプルデータ
        """
        cls.__init_native()
        return _native.MediaUtil_convert_samples(src_channels,
                                                        src_nb_samples,
                                                        src_format,
                                                        src_samples,
                                                        dst_channels,
                                                        dst_nb_samples,
                                                        dst_format)

    @classmethod
    def encode_pixels(cls, width, height, pixel_format, pixels, codec):
        u"""
        指定されたコーデックを用いてピクセルデータをエンコードします。
        :param width:           幅
        :param height:          高さ
        :param pixel_format:    ピクセルフォーマット名
        :param pixels:          ピクセルデータ
        :param codec:           コーデック名
        :return:                エンコード結果のバイナリデータ
        """
        cls.__init_native()
        return _native.MediaUtil_encode_pixels(width, height, pixel_format, pixels, codec)
    
    @classmethod
    def encode_samples(cls, channels, nb_samples, sample_format, samples, codec):
        u"""
        指定されたコーデックを用いてサンプルデータをエンコードします。
        :param channels:        チャンネル数
        :param nb_samples:      チャンネルあたりのサンプル数
        :param sample_format:   サンプルフォーマット名
        :param samples:         サンプルデータ
        :param codec:           コーデック名
        :return:                エンコード結果のバイナリデータ
        """
        cls.__init_native()
        return _native.MediaUtil_encode_samples(channels, nb_samples, sample_format, samples, codec)

    @classmethod
    def blit_pixels(cls,
                    src_pixels, src_stride, src_rect,
                    dst_pixels, dst_stride, dst_pos):
        u"""
        ピクセル領域を転送する
        :param src_pixels:  転送元のピクセルデータ
        :param src_stride:  転送元のピクセルデータのストライド幅
        :param src_rect:    転送元領域(X, Y, 幅, 高さ)
        :param dst_pixels:  転送先のピクセルデータ
        :param dst_stride:  転送先のピクセルデータのストライド幅
        :param dst_pos:     転送先位置(X, Y)
        """
        src_x = src_rect[0]
        src_y = src_rect[1]
        src_width = src_rect[2]
        src_height = src_rect[3]
        dst_x = dst_pos[0]
        dst_y = dst_pos[1]

        for y in range(0, src_height):

            src_index = ((src_y + y) * src_stride + src_x) * 3
            dst_index = ((dst_y + y) * dst_stride + dst_x) * 3
            byte_count = src_width * 3
            dst_pixels[dst_index:dst_index + byte_count] = src_pixels[src_index:src_index + byte_count]

    # @todo 廃止する
    @classmethod
    def ppm_from_rgb24(cls, width, height, pixels):
        u"""
        @deprecated RGB24 形式のピクセルデータを元にして PPM フォーマットのデータを作成します。
        :param width:   幅
        :param height:  高さ
        :param pixels:  RGB24 形式のピクセルデータ
        :return:        PPM フォーマットのデータ
        """
        ppm = bytearray("P6 {0} {1} 255 ".format(width, height).encode('ascii'))
        ppm.extend(pixels)
        return ppm

    # @todo 廃止する
    @classmethod
    def wav_from_s16(cls, channels, sample_rate, samples):
        u"""
        @deprecated S16 形式のサンプルデータを元にして WAV フォーマットのデータを作成します。
        :param channels:    チャンネル数
        :param sample_rate: サンプルレート(Hz)
        :param samples:     S16 形式のサンプルデータ
        :return:            WAV フォーマットのデータ
        """
        wav = bytearray('RIFF'.encode('ascii')) #riff
        wav.extend((len(samples) + 44 - 8).to_bytes(4, 'little'))   #fileSize, int
        wav.extend('WAVE'.encode('ascii'))  # wave
        wav.extend('fmt '.encode('ascii'))  # fmt
        wav.extend(int(16).to_bytes(4, 'little'))  #fmtSize, int
        wav.extend(int(1).to_bytes(2, 'little'))   #fmtId
        wav.extend(channels.to_bytes(2, 'little'))  #channel, short
        wav.extend(sample_rate.to_bytes(4, 'little')) #sampleRate, int
        wav.extend((sample_rate * channels * 2).to_bytes(4, 'little')) #bytePerSec, int
        wav.extend((channels * 2).to_bytes(2, 'little'))   #blockSize, short
        wav.extend(int(16).to_bytes(2, 'little'))  #bitPerSample,short
        wav.extend('data'.encode('ascii'))  #data
        wav.extend(len(samples).to_bytes(4, 'little'))    #samplesSize,int
        wav.extend(samples)

        return wav
