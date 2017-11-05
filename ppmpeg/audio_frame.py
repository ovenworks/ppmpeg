# -*- coding: utf-8 -*-

__author__ = "ppmpeg@ovenworks.jp"

from ppmpeg.media_util import MediaUtil

class AudioFrame:
    u"""
    オーディオフレームクラス
    オーディオフレームの情報を保持するオブジェクトです。
    """
    
    def __init__(self, pts, channels, nb_samples, sample_format, samples):
        u"""
        コンストラクタ
        :param pts:             PTS
        :param channels:        チャンネル数
        :param nb_samples:      チャンネルあたりのサンプル数
        :param sample_format:   サンプルフォーマット名
        :param samples:         サンプルデータ(bytes)
        """
        self.__pts = pts
        self.__channels = channels
        self.__nb_samples = nb_samples
        self.__sample_format = sample_format
        self.__samples = samples

    @property
    def pts(self):
        u"""
        PTSを返します。
        :return:    PTS
        """
        return self.__pts

    @property
    def channels(self):
        u"""
        チャンネル数を返します。
        :return:    チャンネル数
        """
        return self.__channels

    @property
    def nb_samples(self):
        u"""
        チャンネルあたりのサンプル数を返します。
        :return:    チャンネルあたりのサンプル数
        """
        return self.__nb_samples

    @property
    def sample_format(self):
        u"""
        サンプルフォーマットを返します。
        :return:    サンプルフォーマット名
        """
        return self.__sample_format

    @property
    def samples(self):
        u"""
        サンプルデータを返します。
        :return:    サンプルデータ(bytes)
        """        
        return self.__samples

    def converted(self, channels, nb_samples, sample_format):
        u"""
        指定されたチャンネル数、サンプル数とサンプルフォーマットに変換したオーディオフレームを作成して返します。
        :param channels:   チャンネル数
        :param nb_samples: チャンネルあたりのサンプル数
        :sample_format:    サンプルフォーマット名
        :return:           変換したオーディオフレーム
        """
        samples = MediaUtil.convert_samples(self.__channels,
                                            self.__nb_samples,
                                            self.__sample_format,
                                            self.__samples,
                                            channels,
                                            nb_samples,
                                            sample_format)

        return AudioFrame(self.__pts, channels, nb_samples, sample_format, samples)

class ReadedAudioFrame(AudioFrame):
    u"""
    読み出し済みオーディオフレームクラス
    読み出したオーディオフレームの情報を保持するオブジェクトです。
    """
    
    def __init__(self, pts, channels, nb_samples, sample_format, samples, samples_shared):
        u"""
        コンストラクタ
        :see:                   AudioFrame#__init()__
        :param samples_shared:  サンプルデータ領域を他のフレームと共有しているか
        """
        super().__init__(pts, channels, nb_samples, sample_format, samples)
        self.__samples_shared = samples_shared

    @property
    def samples_shared(self):
        u"""
        サンプルデータ領域を他のフレームと共有しているかを返します。
        :return:    共有しているか
        """
        return self.__samples_shared
