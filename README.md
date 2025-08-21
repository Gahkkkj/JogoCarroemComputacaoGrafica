# Jogo de Carro com C++ e OpenGL

Um simples jogo de corrida 3D desenvolvido do zero utilizando C++ e a API gráfica OpenGL. Este projeto foi criado como trabalho final para a disciplina de Computação Gráfica, com o objetivo de aplicar na prática os conceitos fundamentais da renderização 3D em tempo real.

> 🚧 **Status:** Projeto concluído. 🚧

## ✨ Funcionalidades

* 🚗 **Controle do Veículo:** Dirija um carro em um cenário 3D com controles de aceleração, freio e direção.
* 🎥 **Câmera em Terceira Pessoa:** A câmera segue o veículo dinamicamente, mantendo o foco na ação.
* 🏞️ **Renderização de Cenário:** O ambiente é composto por modelos 3D (`.obj`) com texturas aplicadas, como a pista e o gramado.
* 💡 **Pipeline Gráfico Moderno:** Utiliza shaders customizáveis (Vertex e Fragment Shaders em GLSL) para controlar o processo de renderização.
* 🖼️ **Carregamento de Recursos:** O sistema carrega modelos 3D no formato `.obj` e texturas (ex: `.jpg`, `.png`) em tempo de execução.

---

## 🧠 Conceitos de Computação Gráfica Aplicados

Este projeto foi uma oportunidade para implementar e entender os seguintes conceitos:

* **Pipeline Gráfico do OpenGL:** Configuração de janelas (GLFW), gerenciamento de contexto e loop de renderização.
* **Shaders (GLSL):** Criação de Vertex Shaders para transformar os vértices dos modelos e Fragment Shaders para aplicar cores e texturas.
* **Matemáticas 3D (GLM):** Uso de matrizes de Modelo, Visão (View) e Projeção (Projection) para posicionar objetos e a câmera no mundo 3D.
* **Gerenciamento de Câmera:** Implementação de uma câmera que calcula sua posição e orientação a cada frame para seguir o carro.
* **Carregamento de Malhas (Meshes):** Leitura de arquivos `.obj` para extrair vértices, normais e coordenadas de textura.
* **Mapeamento de Texturas:** Aplicação de imagens 2D sobre as superfícies dos modelos 3D.

---

## 🛠️ Tecnologias e Bibliotecas

* **Linguagem:** C++
* **API Gráfica:** [OpenGL](https://www.opengl.org/)
* **Bibliotecas de Suporte:**
    * [**GLFW**](https://www.glfw.org/): Para criação de janelas, contextos OpenGL e gerenciamento de inputs.
    * [**GLEW**](http://glew.sourceforge.net/): Para gerenciamento das extensões do OpenGL.
    * [**GLM**](https://glm.g-truc.net/0.9.9/index.html): Para operações matemáticas com vetores e matrizes.
    * [**stb_image**](https://github.com/nothings/stb/blob/master/stb_image.h): Para carregamento de arquivos de imagem para as texturas.

---

## 🚀 Como Compilar e Executar

O projeto foi configurado para ser compilado com o Visual Studio. As dependências já estão incluídas no repositório.

### Pré-requisitos

* [Visual Studio](https://visualstudio.microsoft.com/pt-br/vs/) com a carga de trabalho **"Desenvolvimento de jogos com C++"** ou **"Desenvolvimento para desktop com C++"**.
* Suporte a **OpenGL 3.3** ou superior na sua placa de vídeo.

### Passos para Execução

```bash
# 1. Clone o repositório
$ git clone [https://github.com/Gahkkkj/JogoCarroemComputacaoGrafica.git](https://github.com/Gahkkkj/JogoCarroemComputacaoGrafica.git)

# 2. Acesse a pasta do projeto
$ cd JogoCarroemComputacaoGrafica
```

3.  **Abra o arquivo da solução** (`CG-Projeto.sln`) com o Visual Studio.
4.  Certifique-se de que a configuração da solução está definida como **Release** e a plataforma como **x86** (ou x64, dependendo de como as bibliotecas foram compiladas).
5.  **Compile o projeto:** Pressione `Ctrl+Shift+B` ou vá em `Build > Build Solution`.
6.  **Execute:** Pressione `F5` ou o botão de play "Local Windows Debugger".

---

## 🎮 Controles

* **W** - Acelerar
* **S** - Frear / Dar ré
* **A** - Virar à esquerda
* **D** - Virar à direita
* **ESC** - Fechar o jogo

---

## 👨‍💻 Autor

| [<img src="https://avatars.githubusercontent.com/u/104975550?v=4" width=115><br><sub>Gabriel da Silva Pereira</sub>](https://github.com/Gahkkkj) |
| :---: |
