import serial
import time

class Xbee():
    def __init__(self, port, baud_rate):
        try:
            self.serial_xbee = serial.Serial(port, baud_rate, timeout=1)
            print(f"[{time.strftime('%H:%M:%S')}] Connexion série établie !")
        except serial.SerialException as e:
            print(f"[{time.strftime('%H:%M:%S')}] Erreur série : {e}")
            exit(1)


    def send_key_value(self, key, value):
        if key.lower() == "exit":
            self.disconnect()
        message = key + ":" + str(value)
        self.serial_xbee.write(message.encode() + b"\n")
        print(f"[{time.strftime('%H:%M:%S')}] Message envoyé : {message}")


    def send_message(self, message):
        if message.lower() == "exit":
            self.disconnect()
        self.serial_xbee.write(message.encode() + b"\n")
        print(f"[{time.strftime('%H:%M:%S')}] Message envoyé : {message}")


    def receive_message(self):
        if self.serial_xbee.in_waiting > 0:
            message = self.serial_xbee.readline().decode('utf-8').strip()
            print(f"[{time.strftime('%H:%M:%S')}] Message reçu : {message}")
            return message
        return None


    def receive_key_value(self):
        message = self.receive_message()
        if message:
            parts = message.split(":", 1)
            if len(parts) == 2:
                key, value = parts[0].strip(), parts[1].strip()
                return key, value
        return None, None


    def disconnect(self):
        print("Fermeture de la connexion...")
        self.serial_xbee.close()
        print("Connexion série fermée.")


if __name__ == "__main__":
    xbee = Xbee("COM7", 9600)
    try:
        while True:
            received_key, received_value = xbee.receive_key_value()
            if received_key:
                print(f"Reçu -> Clé: {received_key}, Valeur: {received_value}")
            
            # input_message = input("Entrez un message : ")
            # xbee.send_message(input_message)

            key = input("Entrez un key : ")
            value = input("Entrez un value : ")
            xbee.send_key_value(key, value)
            received_message = xbee.receive_message()

            time.sleep(1)
    except KeyboardInterrupt:
        xbee.disconnect()
    finally:
        xbee.disconnect()