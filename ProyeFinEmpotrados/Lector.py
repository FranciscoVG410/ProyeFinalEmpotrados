import json 
import time
import serial
import mysql.connector
from mysql.connector import Error

PUERTO_SERIAL = "COM5" #Puerto donde esta el ESP32       
BAUDIOS       = 115200 #Aqui recibe desde el esp
DB_CONFIG = {
    "host": "localhost",
    "user": "root",
    "password": "63690Val",
    "database": "esp32db"
}
# -------------------------------------------------------------------

def conectar_mysql():
    return mysql.connector.connect(**DB_CONFIG)

def insertar_medicion(cur, temp, hum, estado):
    sql = ("INSERT INTO mediciones (temperatura, humedad, estado) "
           "VALUES (%s, %s, %s)")
    cur.execute(sql, (temp, hum, estado))

def main():
    print("Abriendo puerto {} a {} baud…".format(PUERTO_SERIAL, BAUDIOS))
    ser = serial.Serial(PUERTO_SERIAL, BAUDIOS, timeout=2)
    time.sleep(2)  # pequeña pausa para estabilizar el puerto

    try:
        conn = conectar_mysql()
        cur  = conn.cursor()

        while True:
            linea = ser.readline().decode(errors="ignore").strip()
            if not linea:
                continue

            try:
                datos = json.loads(linea)   # ESP32 debería mandar algo como {"temp":24.8,"hum":40.1,"estado":"Condición óptima"}
                temp   = float(datos["temp"])
                hum    = float(datos["hum"])
                estado = str(datos.get("estado", ""))

                insertar_medicion(cur, temp, hum, estado)
                conn.commit()
                print("Guardado:", temp, hum, estado)

            except (ValueError, KeyError, json.JSONDecodeError):
                # Línea malformada – ignórala pero avisa
                print("Línea no JSON:", linea)

    except Error as e:
        print("Error MySQL:", e)

    except serial.SerialException as e:
        print("Error Serial:", e)

    finally:
        try:
            cur.close(); conn.close()
        except: pass
        ser.close()

if __name__ == "__main__":
    main()