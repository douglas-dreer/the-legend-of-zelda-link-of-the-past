Aqui está o documento `enemy-ai.md` estruturado como um guia técnico e de design para o seu projeto de decompilação. Você pode salvá-lo diretamente na raiz do seu repositório ou em uma pasta como `docs/`.

```markdown
# Enemy AI Design & Implementation Guide

Este documento detalha a filosofia, o design e a implementação técnica de Inteligência Artificial (IA) para inimigos e chefes no projeto de decompilação de *The Legend of Zelda: A Link to the Past*. 

O objetivo principal é elevar a dificuldade e o engajamento através de **comportamento tático e adaptativo**, abandonando abordagens preguiçosas como "inimigos esponja" (muito HP) ou "dano absurdo".

---

## 🧠 1. Filosofia de Design: "Smart, Not Sponge"

A inteligência dos inimigos neste projeto é guiada por cinco pilares inspirados em jogos modernos de ação/aventura (como *Tunic*, *Death's Door*, *Hyper Light Drifter*, *Kena* e *Hob*):

1. **Quebra de Padrões e Leitura de Input**: Inimigos que reagem às ações repetitivas do jogador (ex: contra-atacar se o jogador spamma o botão de espada).
2. **Sinergia de Grupo**: Inimigos que trabalham em equipe, protegendo aliados mais frágeis ou flanqueando o jogador.
3. **Movimento Preditivo**: Inimigos que não correm cegamente atrás do jogador, mas sim cortam suas rotas de fuga e antecipam seus movimentos.
4. **Retirada Tática e Auto-preservação**: Inimigos e chefes que, ao estarem com pouca vida, recuam, buscam cura ou chamam reforços, em vez de lutar até a morte de forma burra.
5. **Interação com o Ambiente**: Uso do cenário (destruir pilares, ativar armadilhas da sala) como parte do combate.

---

## 🛠️ 2. Implementação Técnica (C / Engine)

Para implementar essas mecânicas, utilizaremos uma **Máquina de Estados Finitos (FSM)** estendida para os sprites/inimigos. 

### 2.1. Definição de Estados
Adicione novos estados à enumeração de estados do inimigo no código base:

```c
// Exemplo de expansão da enum de estados do Sprite/Inimigo
typedef enum {
    // Estados originais do ALttP
    ENEMY_STATE_IDLE = 0,
    ENEMY_STATE_WALK,
    ENEMY_STATE_ATTACK,
    ENEMY_STATE_HURT,
    ENEMY_STATE_DIE,
    
    // Novos Estados Táticos
    ENEMY_STATE_COUNTER_ATTACK,   // Reage ao ataque do jogador
    ENEMY_STATE_PUNISH_DASH,      // Ataque de área quando o jogador usa o dash
    ENEMY_STATE_SHIELD_ALLY,      // Posiciona-se para proteger um aliado
    ENEMY_STATE_PANIC_RETREAT,    // Foge quando a vida está baixa
    ENEMY_STATE_INTERACT_ENV      // Interage com objetos da sala
} EnemyState;
```

### 2.2. Lógica de Atualização (Update Loop)
A lógica de IA deve ser inserida na função de atualização do sprite (ex: `Sprite_Main` ou uma função específica de AI do inimigo). 

#### A. Quebra de Padrão (Estilo *Tunic*)
```c
void handle_pattern_breaking(Sprite* enemy, Sprite* player) {
    // Se o jogador está atacando e o inimigo pode contra-atacar
    if (player->state == PLAYER_STATE_ATTACKING && enemy->counter_window > 0) {
        enemy->state = ENEMY_STATE_COUNTER_ATTACK;
        enemy->counter_window = 0; // Evita spam de contra-ataque
        enemy->anim_timer = 0;     // Inicia animação de telegrafia
        return;
    }
    
    // Se o jogador está dando dash (esquivando) muito perto
    if (player->state == PLAYER_STATE_DASHING && get_distance(enemy, player) < 48) {
        enemy->state = ENEMY_STATE_PUNISH_DASH;
        return;
    }
}
```

#### B. Retirada Tática (Estilo *Kena* / *BotW*)
```c
void handle_tactical_retreat(Sprite* enemy, Sprite* player) {
    // Verifica se a vida está abaixo de 25%
    if (enemy->hp < (enemy->max_hp >> 2)) { // Divisão por 4 usando shift
        if (!enemy->has_fled) {
            enemy->has_fled = 1;
            enemy->state = ENEMY_STATE_PANIC_RETREAT;
            
            // Torna-o temporariamente invencível ou com hitbox reduzida durante a fuga
            enemy->iframes = 60; 
            
            // Spawna reforços se for um chefe ou elite
            if (enemy->is_elite) {
                spawn_minions(enemy->x_pos, enemy->y_pos, 2);
            }
        }
    }
}
```

#### C. Movimento Preditivo (Estilo *Hyper Light Drifter*)
Em vez de usar a função padrão de perseguição (`move_towards_player`), usamos interceptação vetorial.
```c
void predictive_chase(Sprite* enemy, Sprite* player) {
    // Calcula o tempo estimado para o inimigo alcançar o jogador
    int16_t dist = get_distance(enemy, player);
    int16_t time_to_reach = dist / (enemy->speed + 1);
    
    // Prevê a posição futura do jogador (usando a velocidade atual dele)
    int16_t target_x = player->x_pos + (player->x_vel * (time_to_reach >> 1));
    int16_t target_y = player->y_pos + (player->y_vel * (time_to_reach >> 1));
    
    // Move em direção à posição prevista, não a atual
    move_towards(enemy, target_x, target_y);
}
```

---

## 🎮 3. Adaptação para a Engine do ALttP

Ao modificar o código decompilado do *A Link to the Past*, tenha em mente as particularidades da engine original:

1. **Subpixels e Coordenadas**: O ALttP usa um sistema de coordenadas de 16 bits para posição (`x_pos`, `y_pos`) e 8 bits para subpixels (`x_subpos`, `y_subpos`). Ao calcular distâncias ou prever movimentos, certifique-se de usar as posições completas (ou converter corretamente) para evitar "tremidas" (jitter) nos sprites.
2. **Limites de OAM e Sprites**: O SNES tem um limite rígido de sprites na tela (128 no total, 32 por linha). Inimigos que spawnam muitos projéteis ou partículas podem causar flickering. Limite a quantidade de ações simultâneas.
3. **Colisão (Collision)**: O jogo usa um sistema de colisão baseado em tiles (Room Grid) e caixas delimitadoras (Hitboxes). Para a IA de "cortar caminho", use as funções de pathfinding já existentes no jogo (como as usadas pelos inimigos que andam em labirintos) em vez de criar um A* do zero.
4. **Timers e Frames**: Tudo no ALttP é baseado em frames (60fps). Use contadores de frames (`timer--`) para controlar janelas de contra-ataque e telegrafia, em vez de tempo real em milissegundos.

---

## ⚖️ 4. Regras de Ouro: "Game Feel" e Justiça (Fairness)

Uma IA inteligente pode parecer "injusta" (cheap) se não for bem comunicada ao jogador. Siga estas regras:

*   **Telegrafia é Obrigatória**: Antes de executar um contra-ataque, esquiva ou movimento preditivo, o inimigo **deve** ter uma animação de "wind-up" (preparação), mudar de cor por 5-10 frames, ou emitir um som de alerta. O jogador precisa ter tempo de reação (mínimo de 15-20 frames).
*   **Não "Leia" o Input Cego**: A IA não deve reagir no *exato frame* em que o jogador aperta o botão. Ela deve reagir à *animação* do jogador ou ter um pequeno delay de reação (ex: 5 a 10 frames) para simular "tempo de resposta humano".
*   **Respeite as I-Frames do Jogador**: Se o jogador está em invencibilidade (após levar dano), a IA de perseguição agressiva deve dar um "respiro" (cooldown) para não criar um loop de morte frustrante.
*   **Hitboxes Visíveis**: Durante o desenvolvimento, mantenha a flag de debug de hitboxes ativada. A IA de esquiva e interceptação só funciona se as hitboxes do jogador e do inimigo estiverem perfeitamente calibradas.

---

## 📝 5. Checklist de Implementação

- [ ] Mapear as structs de `Sprite` e `Player` no código atual.
- [ ] Adicionar novas variáveis de estado à struct do inimigo (ex: `counter_window`, `has_fled`).
- [ ] Implementar a FSM (Máquina de Estados) para os novos comportamentos.
- [ ] Criar animações ou usar sprites existentes para telegrafar os novos ataques.
- [ ] Testar a IA em salas pequenas (para testar movimento preditivo) e salas grandes (para testar retirada tática).
- [ ] Ajustar os valores de `timer` e `distance` para garantir que o jogo seja desafiador, mas justo.
```

### Como usar este documento:
1. Salve-o como `enemy-ai.md` na raiz do seu projeto ou dentro de uma pasta `docs/`.
2. Ele serve tanto como documentação para outros desenvolvedores que porventura contribuam com o projeto, quanto como um "norte" para você mesmo não perder o foco na hora de programar as novas mecânicas.
3. Você pode ir adicionando links para as funções específicas do seu código C à medida que for encontrando-as no decompilador (ex: substituindo `move_towards` pelo nome real da função no código do ALttP, como `Sprite_MoveXY` ou similar).