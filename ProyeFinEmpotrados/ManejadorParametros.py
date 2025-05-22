import tkinter as tk
from tkinter import messagebox
import requests


# Dirección IP del dispositivo ESP32 al que se enviarán los parámetros
# modificar si es necesario
ESP32_IP = "http://192.168.100.39"

def enviar_config():
    """
    Función que se ejecuta al presionar el botón de enviar.
    Recoge los valores de temperatura y humedad desde las cajas de texto,
    los convierte a flotantes, y los envía al ESP32 mediante una solicitud POST.
    """
    try:
        temp = float(entry_temp.get())
        hum = float(entry_hum.get())
        data = {
            "tempOpt": temp,
            "humOpt": hum
        }
        response = requests.post(f"{ESP32_IP}/config", json=data)
        if response.status_code == 200:
            messagebox.showinfo("Éxito", "Parámetros enviados correctamente.")
        else:
            messagebox.showerror("Error", f"Código: {response.status_code}")
    except ValueError:
        messagebox.showwarning("Advertencia", "Introduce valores numéricos válidos.")

# Crear ventana
root = tk.Tk()
root.title("Manejador de Parámetros ESP32")

# Etiqueta y campo para ingresar la temperatura óptima
tk.Label(root, text="Temperatura Óptima (°C):").grid(row=0, column=0, padx=10, pady=5)
entry_temp = tk.Entry(root)
entry_temp.grid(row=0, column=1, padx=10, pady=5)

# Etiqueta y campo para ingresar la humedad óptima
tk.Label(root, text="Humedad Óptima (%):").grid(row=1, column=0, padx=10, pady=5)
entry_hum = tk.Entry(root)
entry_hum.grid(row=1, column=1, padx=10, pady=5)

# Botón que ejecuta la función enviar_config al hacer clic
btn_enviar = tk.Button(root, text="Enviar al ESP32", command=enviar_config)
btn_enviar.grid(row=2, column=0, columnspan=2, pady=10)

# Ejecuta el bucle principal de la interfaz gráfica
root.mainloop()