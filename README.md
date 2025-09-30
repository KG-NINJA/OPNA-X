# OPNA-X
VSti
# 📌 OPNA-X（YM2608 正常進化系）VSTiプロトタイプを作成する
# 環境: JUCE + C++17, VST3, Windows（macOS対応も可）
# 機能:
# - 6オペレータFM音源（DX7互換の基本アルゴリズム＋波形選択）
# - 8ch PCM再生（16bit/44.1kHz WAV）
# - SSG拡張（任意波形＋ノイズ＋PWM）
# - MIDI入力でFM＋PCM＋SSGを同時発音可能
# - GUIでFMアルゴリズム選択・PCMマッピング・SSG波形編集
# - JUCEのAudioProcessorで統合し、VST3としてビルド可能

# 実装内容:
# 1. JUCEプロジェクト "OPNA-X" を生成
# 2. Source/FMEngine.{h,cpp} : 6op FMシンセクラス（周波数比、波形、アルゴリズム対応）
# 3. Source/PCMEngine.{h,cpp} : 8ch PCM再生クラス（WAV読み込みとループ対応）
# 4. Source/SSGEngine.{h,cpp} : PSG互換＋任意波形＆ノイズジェネレータ
# 5. PluginProcessor.cpp : MIDIイベントを各音源に振り分け、float32でミックス
# 6. PluginEditor.cpp : アルゴリズム表示、波形描画、PCMマッピングUI
# 7. Resources/pcm/ にデモ用の短いWAVサンプルを配置
# 8. CMakeLists.txt でJUCEとVST3 SDKをリンク

# 条件:
# - コードはビルド可能な最低限の雛形でよい（サウンドが出ることを優先）
# - コメントで各セクションにFM/PCM/SSGの処理ポイントを明記
# - GUIは簡易で構わない（FMアルゴリズム選択、PCMファイル名表示など）
# - Windowsを対象OSとする（macOS対応は後で追加）

# 期待するディレクトリ構成:
# OPNA-X/
# ├─ Source/
# │   ├─ FMEngine.h / .cpp
# │   ├─ PCMEngine.h / .cpp
# │   ├─ SSGEngine.h / .cpp
# │   ├─ PluginProcessor.cpp / .h
# │   └─ PluginEditor.cpp / .h
# ├─ Resources/pcm/
# │   └─ demo_sample.wav
# └─ CMakeLists.txt

# 出力形式:
# - 一式をディレクトリに生成
# - 主要コードファイルはC++17で記述
# - JUCEプロジェクトとしてVisual Studioでそのまま開ける

# ステップ:
# 1. JUCEプロジェクト生成
# 2. 各エンジン（FM, PCM, SSG）の雛形コード作成
# 3. AudioProcessorで統合
# 4. GUIとリソース配置
# 5. CMakeLists生成
