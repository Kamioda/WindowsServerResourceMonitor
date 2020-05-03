# Windows Server Resource Monitor

このプロジェクトは、Windows Serverのリソースの状態を取得するものです。

## ライブラリ等

本リポジトリ内に含まれている以下のものはライブラリ等は次の通りです。

- [AinoMegumi/WindowsServiceCppTemplate](https://github.com/AinoMegumi/WindowsServiceCppTemplate) (MIT License)
- [yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib) (MIT License)(サービス向けに改変)
- [kazuho/picojson](https://github.com/kazuho/picojson) (BSD 2-Clause "Simplified" License)

## プロジェクトのライセンス

このプロジェクトはGPLv3で公開しております。また、各ライブラリのライセンスはLibLicensesフォルダー内にあります。

## データの閲覧方法

かなりお粗末ですが、サーバーには認証システムがついています。サインインをすると、HTTPヘッダーにCookieでアクセストークンが送られてきます。このアクセストークンをリクエストのHTTPヘッダーにCookieで送信することでサーバー側で自動的に認証が行われ、レスポンスが返ってきます。このCookieがない、またはアクセストークンが間違っている場合はエラーが返ってきます

APIについては、[仕様書](https://github.com/AinoMegumi/WindowsServerResourceMonitorSpecification)を確認して下さい

## サービスの制御について

サービスは、アクセストークンを使用して一時停止、再開、停止を行うことができます。

## 現在行っている作業

Windowsサービスの制御機能の実装を行っています
