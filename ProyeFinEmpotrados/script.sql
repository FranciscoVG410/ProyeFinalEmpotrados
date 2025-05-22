CREATE DATABASE IF NOT EXISTS esp32db;
USE esp32db;

CREATE TABLE mediciones (
  id          INT AUTO_INCREMENT PRIMARY KEY,
  temperatura FLOAT,
  humedad     FLOAT,
  estado      VARCHAR(50),
  fecha       TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

select * from mediciones;