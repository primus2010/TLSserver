# simpleTLSserver
Простой однопоточный сервер слушает порт 4433. Принимет соединения защищенные TLS-1.2.
При запуске сервер читает конфигурационный json файл (нужно дать в коммандной строке) вида

    {
      "db_user" : "username",
      "db_password" : "userpass",
      "db_name" : "testdb",
      "cert_file" : "keys/cert.pem",
      "key_file" : "keys/key.pem"
   }
                                                    
Во сремя запуска сервер создает соединение с сервером PostgreSQL, запущенном на этом же хосте. В БД создана таблица users вида

                                 Table "public.users"
       Column   |     Type      | Collation | Nullable |              Default
     -----------+---------------+-----------+----------+-----------------------------------
      id        | integer       |           | not null | nextval('users_id_seq'::regclass)
      firstname | character(64) |           |          |
      lastname  | character(64) |           |          |
      age       | integer       |           |          |



При соединении с сервером  клиент передает ему (серверу) json файл (файл нужно дать в виде параметра) вида

      {
        "users": [
          {
            "firstname":"Ivan",
            "lastname" :"Ivanov",
            "age" : 23
          },
          {
            "firstname":"Petr",
            "lastname" :"Petrov",
            "age" : 32
          },
          {
            "firstname":"Sidor",
            "lastname" :"Sidorov",
            "age" : 25
          }
        ]
      }

или

      {
          "user" : {
            "firstname":"Andrey",
            "lastname" :"Talabuev",
            "age" : 21
          }
      }

и разрывает соединение.

В свою очередь сервер парсит полученную информацию и вставляет ее в описанную таблицу.
Сейчас сервер запускается из терминала и выводит в терминал.
Что можно добавить: сделать сервер демоном и выводить информацию о работе в syslog. Можно запустить как сервис.

Еще про сервер и клиент. Сервер в качестве транспорта использует tcp сокет, клиент -- BIO из OpenSSL.

