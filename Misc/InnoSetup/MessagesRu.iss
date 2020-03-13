[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[CustomMessages]
TypesClient=Клиентская
TypesServer=Серверная
TypesFull=Полная (новый кластер)
TypesCustom=Выборочная
TypesDefault=Предустановленная
TypesProgram=Программа

ComponentsClaster=База данных нового кластера
ComponentsUpdate=Сервис обновлений
ComponentsServer=Новый сервер кластера
ComponentsProgramFiles=Программные компоненты для работы пользователей

strArmSuffix= АРМ

msgNeedNet35=Для работы программы необходим .Net 3.5. Установка невозможна.
msgNeedRedist10=Для работы программы необходимо установить Распространяемый пакет Microsoft Visual C++ 2010 (x86).
msgNeedRedist12=Для работы программы необходимо установить Распространяемый пакет Microsoft Visual C++ 2013 (x86).

msgInstallPostgres=Установка PostgreSQL 9
msgRunPostgres92=Подготовка и запуск PostgreSQL 9
msgStopPostgres92=Остановка и подготовка к удалению PostgreSQL 9
msgPostgres92InstallerMissed=На установочном носителе отсутствует установщик PostgreSQL 9.2 (postgresql-9.2.6-3-windows.exe). Установка кластера невозможна.
msgPostgres92DirectoryExists=Установка базы данных PostgreSQL невозможна, т.к. конечный каталог уже существует. Возможно БД кластера уже установлен в данный каталог. Каталог: 

msgInstallServer=Установка сервера
msgStartServer=Запуск сервера
msgStopServer=Остановка сервера

msgClasterOptionsCaption=Настройка БД кластера
msgClasterOptionsDescription=Необходимо ввести данные настройки БД кластера
msgClasterOptionsSubCaption=Введите DNS имя или IP адрес данного сервера, которые будет использоваться %nдля подключения к нему других  серверов кластера.
msgClasterOptionsSubCaption2=Установите свободный порт для БД (порт по умолчанию  для PostgreSQL: 5432, %nесли у Вас уже установлен(ы) БД PostgreSQL, то необходимо  изменить данное %nзначение).
msgClasterOptionsSubCaption3=Введённое подключение необходимо разрешить в настройках Вашего Firewall.
msgClasterOptionsServer=IP/DNS имя
msgClasterOptionsPort=Номер порта
msgClasterOptionsPortState=Состояние:
msgClasterDataCaption=База данных кластера
msgClasterDataDescription=Расположение на диске папки базы данных кластера
msgClasterDataInfo=Путь:
msgClasterServerInvalid=Введённое имя сервера не может быть разрешено ни как DNS имя, ни как IP адрес, выберете другое значение.
msgClasterPortInvalid=Введённый порт занят, введите другое значение.

msgServerOptionsCaption=Настройка сервера
msgServerOptionsDescription=Необходимо ввести данные для настройки сервера
msgServerOptionsSubCaption=Введите IP адрес данного сервера, который будет доступен для %nподключения к нему других объектов кластера.
msgServerOptionsSubCaption2=Введите имя сервера в кластере.
msgServerOptionsSubCaption3=Введите Уникальный Идентификатор сервера.
msgServerOptionsIp=IP сервера
msgServerOptionsName=Имя сервера
msgServerOptionsGuid=УИд сервера

msgGuidGenerateFail=Ошибка создания Уникального Идентификатора. Необходимо устранить проблему вручную.%nУстановлен не уникальный идентификатор 'xxx'.

msgShowUserKeys=Вы успешно установили БД кластера.%nПодключение к БД защищается на уровне прав самой БД. Для этого сгенерировано 2 ключа: 'connect1.ini' и 'connect2.ini'. При взломе пароля первого ключа у злоумышленника на уровне БД не будет прав, чтобы принести непоправимый вред, поэтому рекомендуется использовать второй ключ только для устаноки сервера кластера и для прикладного ПО администратора кластера. Для пользователей кластера рекомендуется использовать первый ключ.%n%nРасположение ключей: %n
msgCopyConnect=Копирование файлов подключения к кластеру
msgCopyConnectCaption=Выберете файл подключения
msgServerInitError=Неудалось создать .ini файл настройки сервера
msgArmInitError=Неудалось создать .ini файл настройки АРМ
msgCopyConnectManual=Копирование файла подключения было отменено, сервер не будет функционировать пока файл не будет скопирован вручную.
msgCopyError=Ошибка при копировании файла.
msgCopyLicCaption=Выбор файла лицензионного ключа
msgLicFilter=Файлы настроек (*.ini)|*.ini|Все файлы|*.*
msgCopyLicManual=Копирование файла лицензионного ключа было отменено, сервер не будет функционировать пока файл не будет скопирован вручную.
