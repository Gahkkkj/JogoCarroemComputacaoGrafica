

// Inclusão de bibliotecas padrão do C++
#include <iostream> // Para entrada/saída
#include <string>   // Para manipulação de strings
#include <assert.h> // Para verificações de assert
#include <cmath>    // Para funções matemáticas
#include <cstdlib>  // Para funções gerais (como rand)
#include <ctime>    // Para funções de tempo

using namespace std;

// Bibliotecas para OpenGL e GLAD (gerenciador de extensões OpenGL)
#include <glad/glad.h> // Necessário incluir antes do GLFW

// Biblioteca GLFW para criação de janelas e contexto OpenGL
#include <GLFW/glfw3.h>

// Biblioteca GLM para matemática em gráficos (vetores, matrizes, etc.)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm; // Usa o namespace glm para facilitar

// Bibliotecas para manipulação de imagens
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h> // Para carregar texturas

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h> // Para salvar imagens

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h> // Para renderização de texto

// Enumeração para os estados do jogo
enum EstadoJogo
{
    MENU,       // Estado do menu inicial
    JOGANDO,    // Estado durante o jogo
    FIM_DE_JOGO // Estado quando o jogo termina
};
EstadoJogo estadoJogo = MENU; // Estado inicial

// Estrutura para representar sprites no jogo
struct Sprite
{
        
    GLuint VAO;        // Vertex Array Object (configuração de vértices)
    GLuint idTextura;  // ID da textura OpenGL
    vec3 posicao;      // Posição no espaço 3D (x,y,z)
    vec3 dimensoes;    // Escala (largura, altura, profundidade)
    float angulo;      // Rotação
    float velocidade;  // Velocidade de movimento
    int numAnimacoes;  // Número de animações disponíveis
    int numQuadros;    // Número de quadros por animação
    int quadroAtual;   // Quadro atual da animação
    int animacaoAtual; // Animação atual
    float ds, dt;      // Deslocamento de textura (para animação)
};

// Estrutura para botões no menu
struct Botao
{
    vec2 posicao; // Posição (x,y) na tela
    vec2 tamanho; // Largura e altura
    string texto; // Texto do botão
    vec3 cor;     // Cor RGB
};

// classes de funções (declarações antes da implementação)
void tecladoCallbackMenu(GLFWwindow *janela, int tecla, int scancode, int acao, int modo);
void mouseCallbackMenu(GLFWwindow *janela, int botao, int acao, int mods);
int configurarShader();
int configurarSprite(int numAnimacoes, int numQuadros, float &ds, float &dt);
int carregarTextura(string caminhoArquivo);
void drawSprite(GLuint idShader, Sprite sprite, bool usarCorSolida = false);
bool verificarColisao(const Sprite &a, const Sprite &b);
void renderizarMenu(GLuint idShader);
bool mouseSobreBotao(vec2 posicaoMouse, Botao botao);

// Constantes de configuração do jogo
const GLuint LARGURA = 800, ALTURA = 600; // Dimensões da janela

// Configurações de dificuldade e gameplay
const int NUM_TEXTURAS_CARROS = 4;              // Número de texturas diferentes para carros inimigos
const float VELOCIDADE_INIMIGO_BASE = 2.0f;     // Velocidade inicial dos inimigos
const float VELOCIDADE_INIMIGO_MAXIMA = 7.0f;   // Velocidade máxima dos inimigos
const float TAXA_AUMENTO_DIFICULDADE = 0.6f;    // Quanto aumenta a velocidade por segundo
const float INTERVALO_DIFICULDADE = 8.0f;      // Intervalo para aumentar dificuldade
const int MAX_INIMIGOS = 1000;                   // Número máximo de inimigos na tela
const float INTERVALO_APARICAO_INIMIGOS = 0.3f; // Intervalo entre aparecer novos inimigos

// Configurações de interface do menu
const vec2 TAMANHO_BOTAO = vec2(200, 60);              // Tamanho padrão dos botões
const float OFFSET_Y_BOTAO = 50.0f;                    // Espaçamento vertical entre botões
const vec3 COR_BOTAO = vec3(0.2f, 0.3f, 0.8f);         // Cor base dos botões
const vec3 COR_HOVER_INICIAR = vec3(1.0f, 0.5f, 0.5f); // Cor quando mouse está sobre o botão Iniciar
const vec3 COR_HOVER_SAIR = vec3(0.8f, 0.5f, 0.8f);    // Cor quando mouse está sobre o botão Sair

// Array de botões do menu
Botao botoes[] = {
    {vec2(LARGURA / 2, ALTURA / 2 + OFFSET_Y_BOTAO), TAMANHO_BOTAO, "Iniciar", vec3(1.0f, 0.0f, 0.0f)}, // Botão Iniciar (vermelho)
    {vec2(LARGURA / 2, ALTURA / 2 - OFFSET_Y_BOTAO), TAMANHO_BOTAO, "Sair", vec3(0.5f, 0.0f, 0.5f)}};   // Botão Sair (roxo)

// Código fonte dos shaders (programas que rodam na GPU)
const GLchar *codigoFonteVertexShader = R"(
    #version 400
    layout (location = 0) in vec2 position;
    layout (location = 1) in vec2 texc;
    
    uniform mat4 projection;
    uniform mat4 model;
    out vec2 tex_coord;
    void main()
    {
        tex_coord = vec2(texc.s,1.0-texc.t);
        gl_Position = projection * model * vec4(position, 0.0, 1.0);
    }
)"; // Vertex Shader (processa vértices)
const GLchar *fragmentShaderSource = R"(
    #version 400
    in vec2 tex_coord;
    out vec4 color;
    uniform sampler2D tex_buff;
    uniform vec2 offset_tex;
    uniform bool useSolidColor;
    uniform vec3 solidColor;
    
    void main()
    {
        if (useSolidColor) {
            color = vec4(solidColor, 1.0);
        } else {
            color = texture(tex_buff, tex_coord + offset_tex);
        }
    }
)"; // Fragment Shader (processa pixels)

// configuraçoes fixas
bool teclas[1024];                         // Array para estado das teclas (pressionadas ou não)
float FPS = 12.0;                          // Frames por segundo para animação
float ultimoTempo = 0.0;                   // Controle de tempo para animação
GLuint VAOTexto, VBOTexto;                 // Buffers para renderização de texto
GLuint texturaFonte;                       // Textura da fonte
stbtt_bakedchar dadosCaracteres[96];       // Dados dos caracteres da fonte
Sprite inimigos[MAX_INIMIGOS];             // Array de inimigos
float temporizadorAparecerInimigos = 0.0f; // Contador para aparecer novos inimigos
Sprite fundo, jogador;                     // Sprites do fundo e jogador
GLFWwindow *janela;                        // Ponteiro para a janela GLFW

// Implementação das funções

// Inicializa os inimigos com texturas aleatórias
void inicializarInimigos()
{
    // Carrega as texturas dos carros inimigos
    GLuint texturasCarros[NUM_TEXTURAS_CARROS];
    texturasCarros[0] = carregarTextura("../assets/sprites/carro1.png");
    texturasCarros[1] = carregarTextura("../assets/sprites/carro2.png");
    texturasCarros[2] = carregarTextura("../assets/sprites/carro3.png");
    texturasCarros[3] = carregarTextura("../assets/sprites/carro4.png");

    // Configura cada inimigo
    for (int i = 0; i < MAX_INIMIGOS; i++)
    {
        inimigos[i].VAO = configurarSprite(1, 1, inimigos[i].ds, inimigos[i].dt);
        int tipoCarro = rand() % NUM_TEXTURAS_CARROS; // Escolhe textura aleatória
        inimigos[i].idTextura = texturasCarros[tipoCarro];
        inimigos[i].dimensoes = vec3(100.0f, 100.0f, 1.0f); // Tamanho padrão
        inimigos[i].posicao = vec3(-100.0f, -100.0f, 0.0f); // Posição inicial fora da tela
        inimigos[i].velocidade = VELOCIDADE_INIMIGO_BASE;   // Velocidade inicial
        inimigos[i].numAnimacoes = 1;                       // Sem animações
        inimigos[i].numQuadros = 1;                         // Apenas 1 quadro
        inimigos[i].animacaoAtual = 0;
        inimigos[i].quadroAtual = 0;
    }
}

// Atualiza a posição e comportamento dos inimigos
void atualizarInimigos(float deltaTempo)
{
    if (estadoJogo != JOGANDO) // Só atualiza se estiver jogando
        return;

    // Carrega texturas dos carros (apenas na primeira vez)
    static GLuint texturasCarros[NUM_TEXTURAS_CARROS];
    static bool texturasCarregadas = false;
    if (!texturasCarregadas)
    {
        texturasCarros[0] = carregarTextura("../assets/sprites/carro1.png");
        texturasCarros[1] = carregarTextura("../assets/sprites/carro2.png");
        texturasCarros[2] = carregarTextura("../assets/sprites/carro3.png");
        texturasCarros[3] = carregarTextura("../assets/sprites/carro4.png");
        texturasCarregadas = true;
    }

    // Atualiza temporizadores
    temporizadorAparecerInimigos += deltaTempo;
    static float tempoJogo = 0.0f;
    tempoJogo += deltaTempo;

    // Aumenta dificuldade com o tempo
    float velocidadeInimigoAtual = VELOCIDADE_INIMIGO_BASE + (tempoJogo * TAXA_AUMENTO_DIFICULDADE);
    if (velocidadeInimigoAtual > VELOCIDADE_INIMIGO_MAXIMA)
    {
        velocidadeInimigoAtual = VELOCIDADE_INIMIGO_MAXIMA;
    }

    // Aparece novo inimigo se passou o intervalo
    if (temporizadorAparecerInimigos >= INTERVALO_APARICAO_INIMIGOS)
    {
        temporizadorAparecerInimigos = 0.0f;
        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            // Encontra um inimigo que não está na tela
            if (inimigos[i].posicao.y < -50.0f)
            {
                // Posiciona em um lugar aleatório no topo da tela
                inimigos[i].posicao.x = 100.0f + static_cast<float>(rand() % 600);
                inimigos[i].posicao.y = ALTURA + 50.0f;
                inimigos[i].velocidade = velocidadeInimigoAtual;
                int tipoCarro = rand() % NUM_TEXTURAS_CARROS;
                inimigos[i].idTextura = texturasCarros[tipoCarro];
                break;
            }
        }
    }

    // Move todos os inimigos ativos
    for (int i = 0; i < MAX_INIMIGOS; i++)
    {
        if (inimigos[i].posicao.y > -50.0f) // Se está na tela
        {
            inimigos[i].posicao.y -= inimigos[i].velocidade; // Move para baixo
            if (inimigos[i].posicao.y < -50.0f)              // Saiu da tela
            {
                inimigos[i].posicao = vec3(-100.0f, -100.0f, 0.0f); // Remove
            }
        }
    }
}

// Desenha todos os inimigos na tela
void drawInimigos(GLuint idShader)
{
    if (estadoJogo != JOGANDO) // Só desenha se estiver jogando
        return;

    for (int i = 0; i < MAX_INIMIGOS; i++)
    {
        if (inimigos[i].posicao.y > -50.0f) // Se está na tela
        {
            // Calcula deslocamento de textura para animação
            float ds = inimigos[i].quadroAtual * inimigos[i].ds;
            float dt = inimigos[i].animacaoAtual * inimigos[i].dt;
            glUniform2f(glGetUniformLocation(idShader, "offset_tex"), ds, dt);
            drawSprite(idShader, inimigos[i]); // Desenha o inimigo
        }
    }
}

// Verifica colisão entre dois sprites usando bounding boxes
bool verificarColisao(const Sprite &a, const Sprite &b)
{
    // Calcula os limites do sprite A
    float aEsquerda = a.posicao.x - a.dimensoes.x * 0.2f;
    float aDireita = a.posicao.x + a.dimensoes.x * 0.2f;
    float aTopo = a.posicao.y + a.dimensoes.y * 0.2f;
    float aBase = a.posicao.y - a.dimensoes.y * 0.2f;

    // Calcula os limites do sprite B
    float bEsquerda = b.posicao.x - b.dimensoes.x * 0.2f;
    float bDireita = b.posicao.x + b.dimensoes.x * 0.2f;
    float bTopo = b.posicao.y + b.dimensoes.y * 0.2f;
    float bBase = b.posicao.y - b.dimensoes.y * 0.2f;

    // Verifica sobreposição nas duas dimensões
    return (aDireita > bEsquerda && aEsquerda < bDireita && aTopo > bBase && aBase < bTopo);
}

// Verifica se o mouse está sobre um botão
bool mouseSobreBotao(vec2 posicaoMouse, Botao botao)
{
    return (posicaoMouse.x > botao.posicao.x - botao.tamanho.x / 2 &&
            posicaoMouse.x < botao.posicao.x + botao.tamanho.x / 2 &&
            posicaoMouse.y > botao.posicao.y - botao.tamanho.y / 2 &&
            posicaoMouse.y < botao.posicao.y + botao.tamanho.y / 2);
}

// Renderiza o menu com os botões
void renderizarMenu(GLuint idShader)
{
    // Limpa a tela com cor escura
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Desenha cada botão
    for (int i = 0; i < 2; i++)
    {
        Sprite spriteBotao;
        spriteBotao.VAO = configurarSprite(1, 1, spriteBotao.ds, spriteBotao.dt);
        spriteBotao.posicao = vec3(botoes[i].posicao.x, botoes[i].posicao.y, 0);
        spriteBotao.dimensoes = vec3(botoes[i].tamanho.x, botoes[i].tamanho.y, 1);
        spriteBotao.idTextura = 0;

        // Obtém posição do mouse
        double xpos, ypos;
        glfwGetCursorPos(janela, &xpos, &ypos);
        ypos = ALTURA - ypos; 
        bool mouseSobre = mouseSobreBotao(vec2(xpos, ypos), botoes[i]);

        // Define cor do botão normal e hover
        vec3 corBotao = botoes[i].cor;
        if (mouseSobre)
        {
            if (i == 0)                       // Botão PLAY
                corBotao = COR_HOVER_INICIAR; // Vermelho claro
            else                              // Botão SAIR
                corBotao = COR_HOVER_SAIR;    // Roxo claro
        }

        // Define a cor uniforme no shader
        glUniform3f(glGetUniformLocation(idShader, "solidColor"),
                    corBotao.r, corBotao.g, corBotao.b);
        drawSprite(idShader, spriteBotao, true); 
    }
}

// Callback para clique do mouse no menu
void mouseCallbackMenu(GLFWwindow *janela, int botao, int acao, int mods)
{
    if (estadoJogo == MENU && botao == GLFW_MOUSE_BUTTON_LEFT && acao == GLFW_PRESS)
    {
        // Obtém posição do mouse
        double xpos, ypos;
        glfwGetCursorPos(janela, &xpos, &ypos);
        ypos = ALTURA - ypos; // Inverte eixo Y

        // Verifica cada botão
        for (int i = 0; i < 2; i++)
        {
            if (mouseSobreBotao(vec2(xpos, ypos), botoes[i]))
            {
                if (i == 0) // Botão Iniciar
                {
                    estadoJogo = JOGANDO;                // Muda para estado de jogo
                    jogador.posicao = vec3(300, 100, 0); // Reposiciona jogador
                }
                else // Botão Sair
                {
                    glfwSetWindowShouldClose(janela, GL_TRUE); // Fecha o jogo
                }
            }
        }
    }
}

// Callback para teclado (usado no menu e no jogo)
void tecladoCallbackMenu(GLFWwindow *janela, int tecla, int scancode, int acao, int modo)
{
    // Tecla ESC - volta para o menu ou sai
    if (tecla == GLFW_KEY_ESCAPE && acao == GLFW_PRESS)
    {
        if (estadoJogo == JOGANDO)
        {
            estadoJogo = MENU; // Volta ao menu
        }
        else
        {
            glfwSetWindowShouldClose(janela, GL_TRUE); // Sai do jogo
        }
    }

    // Atualiza array de teclas pressionadas
    if (acao == GLFW_PRESS)
    {
        teclas[tecla] = true;
        // Enter no menu inicia o jogo
        if (estadoJogo == MENU && tecla == GLFW_KEY_ENTER)
        {
            estadoJogo = JOGANDO;
            jogador.posicao = vec3(300, 100, 0);
            // Reseta posição dos inimigos
            for (int j = 0; j < MAX_INIMIGOS; j++)
            {
                inimigos[j].posicao = vec3(-100.0f, -100.0f, 0.0f);
            }
        }
    }
    else if (acao == GLFW_RELEASE)
    {
        teclas[tecla] = false;
    }
}

// Configura e compila os shaders
int configurarShader()
{
    // Cria e compila o vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &codigoFonteVertexShader, NULL);
    glCompileShader(vertexShader);

    // Verifica erros de compilação
    GLint sucesso;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &sucesso);


    // Cria e compila o fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &sucesso);


    // Cria o programa de shader e vincula os shaders
    GLuint programaShader = glCreateProgram();
    glAttachShader(programaShader, vertexShader);
    glAttachShader(programaShader, fragmentShader);
    glLinkProgram(programaShader);
    glGetProgramiv(programaShader, GL_LINK_STATUS, &sucesso);

    // Limpa os shaders depois de vinculados
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return programaShader;
}

// Configura um sprite com VAO e VBO
int configurarSprite(int numAnimacoes, int numQuadros, float &ds, float &dt)
{
    // Calcula deslocamento de textura para animação
    ds = 1.0 / (float)numQuadros;
    dt = 1.0 / (float)numAnimacoes;

    // Define os vértices do quadrado 
    GLfloat vertices[] = {
        // x    y    s     t
        -0.5,  0.5, 0.0,  dt, // Topo esquerdo
        -0.5, -0.5, 0.0, 0.0, // Base esquerda
         0.5,  0.5,  ds,  dt, // Topo direito
        -0.5, -0.5, 0.0, 0.0, // Base esquerda 
         0.5, -0.5,  ds, 0.0, // Base direita
         0.5,  0.5,  ds, dt}; // Topo direito 

    // Cria e configura o VBO
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Cria e configura o VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    // Atributo 0 - Posição
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    // Atributo 1 - Coordenadas de textura
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // Desvincula buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

// Reinicia o jogo para o estado inicial
void reiniciarJogo()
{
    estadoJogo = MENU;                   // Volta para o menu
    jogador.posicao = vec3(300, 100, 0); // Reposiciona jogador
    // Remove todos os inimigos
    for (int j = 0; j < MAX_INIMIGOS; j++)
    {
        inimigos[j].posicao = vec3(-100.0f, -100.0f, 0.0f);
    }
    temporizadorAparecerInimigos = 0.0f; // Reseta temporizador
}

// Desenha um sprite na tela
void drawSprite(GLuint idShader, Sprite sprite, bool usarCorSolida)
{
    // Define se usa cor sólida ou textura
    glUniform1i(glGetUniformLocation(idShader, "useSolidColor"), usarCorSolida ? GL_TRUE : GL_FALSE);
    glBindVertexArray(sprite.VAO); // usa cor sólida se não tiver textura
    if (!usarCorSolida)
    {
        glBindTexture(GL_TEXTURE_2D, sprite.idTextura); // Vincula textura se não for cor sólida
    }
    // Calcula matriz de modelo (posição, escala)
    mat4 modelo = mat4(1);
    modelo = translate(modelo, sprite.posicao);
    modelo = scale(modelo, sprite.dimensoes);
    // Passa matriz para o shader
    glUniformMatrix4fv(glGetUniformLocation(idShader, "model"), 1, GL_FALSE, value_ptr(modelo));
    // Desenha os triângulos
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0); // Desvincula VAO
}

// Carrega uma textura de arquivo
int carregarTextura(string caminhoArquivo)
{
    GLuint idTextura;
    glGenTextures(1, &idTextura); // Gera ID da textura
    glBindTexture(GL_TEXTURE_2D, idTextura);

    // Configura parâmetros da textura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Carrega imagem do arquivo
    int largura, altura, nrCanais;
    unsigned char *dados = stbi_load(caminhoArquivo.c_str(), &largura, &altura, &nrCanais, 0);

    if (dados)
    {
        // Carrega dados na GPU baseado no número de canais (RGB ou RGBA)
        if (nrCanais == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, largura, altura, 0, GL_RGB, GL_UNSIGNED_BYTE, dados);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, largura, altura, 0, GL_RGBA, GL_UNSIGNED_BYTE, dados);
        }
        glGenerateMipmap(GL_TEXTURE_2D); 
    }
    else
    {
        cout << "Falha ao carregar textura" << endl;
    }

    // Libera memória da imagem
    stbi_image_free(dados);
    glBindTexture(GL_TEXTURE_2D, 0); // Desvincula textura

    return idTextura;
}

// Função principal
int main()
{
    // Inicializa GLFW
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 8); // Habilita anti-aliasing

    // Inicializa array de teclas
    for (int i = 0; i < 1024; i++)
    {
        teclas[i] = false;
    }

    // Inicializa gerador de números aleatórios
    srand(static_cast<unsigned int>(time(nullptr)));

    // Cria janela GLFW
    janela = glfwCreateWindow(LARGURA, ALTURA, "Meu Jogo", nullptr, nullptr);
    if (!janela)
    {
        cerr << "Falha ao criar a janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(janela);
    // Configura callbacks
    glfwSetKeyCallback(janela, tecladoCallbackMenu);
    glfwSetMouseButtonCallback(janela, mouseCallbackMenu);

    // Inicializa GLAD (carrega funções OpenGL)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cerr << "Falha ao inicializar GLAD" << endl;
        return -1;
    }

    // Configura viewport
    int largura, altura;
    glfwGetFramebufferSize(janela, &largura, &altura);
    glViewport(0, 0, largura, altura);

    // Configura shaders
    GLuint idShader = configurarShader();

    // Configuração do fundo
    fundo.VAO = configurarSprite(1, 1, fundo.ds, fundo.dt);
    fundo.idTextura = carregarTextura("../assets/tex/1.png");
    fundo.posicao = vec3(400, 300, 0);   // Centro da tela
    fundo.dimensoes = vec3(800, 600, 1); // Cobre toda a tela
    fundo.angulo = 0.0;

    // Configuração do jogador
    jogador.VAO = configurarSprite(1, 1, jogador.ds, jogador.dt);
    jogador.idTextura = carregarTextura("../assets/sprites/player.png");
    jogador.posicao = vec3(300, 100, 0);            // Posição inicial
    jogador.dimensoes = vec3(100.0f, 100.0f, 1.0f); // Tamanho
    jogador.velocidade = 3.0;                       // Velocidade de movimento
    jogador.numAnimacoes = 1;                       // Sem animações
    jogador.numQuadros = 1;                         // Apenas 1 quadro
    jogador.angulo = 0.0;
    jogador.animacaoAtual = 0;
    jogador.quadroAtual = 0;

    // Inicializa inimigos
    inicializarInimigos();

    // Configura shader e textura
    glUseProgram(idShader);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(idShader, "tex_buff"), 0);

    // Configura matriz de projeção ortográfica do vertexshader
    mat4 projecao = ortho(0.0, 800.0, 0.0, 600.0, -1.0, 1.0);
    glUniformMatrix4fv(glGetUniformLocation(idShader, "projection"), 1, GL_FALSE, value_ptr(projecao));

    // Configura blending e depth test
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    // Variáveis para controle de tempo e FPS
    double ultimoFrame = glfwGetTime();
    double tempoInicial = glfwGetTime();

    // Loop principal do jogo
    while (!glfwWindowShouldClose(janela))
    {
        // Calcula delta time
        double frameAtual = glfwGetTime();
        float deltaTempo = static_cast<float>(frameAtual - ultimoFrame);
        ultimoFrame = frameAtual;

        // Atualiza título da janela com FPS e tempo
        double tempoAtual = glfwGetTime() - tempoInicial;
        double fps = 1.0 / deltaTempo;
        char tituloJanela[128];
        sprintf(tituloJanela, "Tempo: %.1fs | FPS: %.1f", tempoAtual, fps);
        glfwSetWindowTitle(janela, tituloJanela);

        // Processa eventos
        glfwPollEvents();

        // Limpa buffers
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Máquina de estados do jogo
        switch (estadoJogo)
        {
        case MENU:
            renderizarMenu(idShader); // Desenha menu
            break;

        case JOGANDO: //Configuração de teclas W,A,S,D e setas do teclado
        {
            if (teclas[GLFW_KEY_UP] || teclas[GLFW_KEY_W])
            {
                jogador.posicao.y += jogador.velocidade;
                if (jogador.posicao.y > ALTURA - jogador.dimensoes.y / 2)
                    jogador.posicao.y = ALTURA - jogador.dimensoes.y / 2;
            }
            if (teclas[GLFW_KEY_DOWN] || teclas[GLFW_KEY_S])
            {
                jogador.posicao.y -= jogador.velocidade;
                if (jogador.posicao.y < jogador.dimensoes.y / 2)
                    jogador.posicao.y = jogador.dimensoes.y / 2;
            }
            if (teclas[GLFW_KEY_LEFT] || teclas[GLFW_KEY_A])
            {
                jogador.posicao.x -= jogador.velocidade;
                if (jogador.posicao.x < jogador.dimensoes.x / 2)
                    jogador.posicao.x = jogador.dimensoes.x / 2;
            }
            if (teclas[GLFW_KEY_RIGHT] || teclas[GLFW_KEY_D])
            {
                jogador.posicao.x += jogador.velocidade;
                if (jogador.posicao.x > LARGURA - jogador.dimensoes.x / 2)
                    jogador.posicao.x = LARGURA - jogador.dimensoes.x / 2;
            }

            // Desenha fundo
            glUniform2f(glGetUniformLocation(idShader, "offset_tex"), 0.0, 0.0);
            drawSprite(idShader, fundo);

            // Atualiza e desenha inimigos
            atualizarInimigos(deltaTempo);
            drawInimigos(idShader);

            // Desenha jogador
            float ds = jogador.quadroAtual * jogador.ds;
            float dt = jogador.animacaoAtual * jogador.dt;
            glUniform2f(glGetUniformLocation(idShader, "offset_tex"), ds, dt);
            drawSprite(idShader, jogador);

            // Atualiza animação do jogador
            float agora = glfwGetTime();
            float deltaTempoAnim = agora - ultimoTempo;
            if (deltaTempoAnim >= 1 / FPS)
            {
                jogador.quadroAtual = (jogador.quadroAtual + 1) % jogador.numQuadros;
                ultimoTempo = agora;
            }

            // Verifica colisões se aconteceu da GAME OVER
            for (int i = 0; i < MAX_INIMIGOS; i++)
            {
                if (inimigos[i].posicao.y > -50.0f && verificarColisao(jogador, inimigos[i]))
                {
                    estadoJogo = FIM_DE_JOGO; // Colisão detectada
                    break;
                }
            }
            break;
        }

        case FIM_DE_JOGO:
        {
            // Temporizador para voltar ao menu após colisão
            static float temporizadorFimJogo = 0.0f;
            temporizadorFimJogo += deltaTempo;

            // Desenha fundo
            glUniform2f(glGetUniformLocation(idShader, "offset_tex"), 0.0, 0.0);
            drawSprite(idShader, fundo);

            // Depois de 1 segundo, volta para o menu
            if (temporizadorFimJogo >= 1.0f)
            {
                reiniciarJogo();
                temporizadorFimJogo = 0.0f;
            }
            break;
        }
        }

        // Troca buffers e verifica eventos
        glfwSwapBuffers(janela);
    }

    // Finaliza GLFW
    glfwTerminate();
    return 0;
}