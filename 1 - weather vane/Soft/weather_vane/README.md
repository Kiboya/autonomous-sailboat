# Guide d'installation et de configuration pour ESP32C3

## Utilisation du débogueur sur ESP32C3

Cette documentation explique comment configurer et utiliser le débogueur pour ESP32C3. Elle est divisée en plusieurs sections pour faciliter la navigation et l'installation.

## Récupération de l'environnement de dev et configuration

```bash
git clone git@github.com:Kiboya/autonomous-sailboat.git
```

Installation des dépendances du projet

```bash
sudo apt install -y clang-format doxygen graphviz make npm
```

Installation de mdl (markdown lint)

```bash
sudo npm cache clean -f && sudo npm install -g n && sudo n stable
sudo npm install -g markdownlint-cli
```

## Documentation disponible

1. **[Configuration de l'environnement ESP-IDF et OpenOCD](docs/esp-idf-setup.md)**  
   Cette documentation couvre les étapes nécessaires pour installer et configurer ESP-IDF, OpenOCD, et le débogueur GDB pour ESP32C3.

2. **[Configuration de Visual Studio Code pour ESP32C3](docs/vscode-setup.md)**  
   Cette documentation (à venir) expliquera comment configurer Visual Studio Code pour le développement et le débogage avec ESP32C3.

3. **[Résultats et analyses](docs/result.md)**  
   Cette documentation présente les résultats, analyses et observations liés au projet.

