# Discovery Log - The Legend of Zelda: A Link to the Past

## Índice de Descobertas

| Data | Descoberta | Severidade | Status |
|------|------------|------------|--------|
| 2024-07-17 | [D001](#d001---binary-rebuilder-nao-processa-db-string) | Crítica | Resolvido |
| 2024-07-17 | [D002](#d002---disassembler-interpreta-dados-como-brk) | Baixa | Conhecido |

---

## D001 - Binary Rebuilder não processa .db "string"

### Data: 2024-07-17
### Severidade: CRÍTICA
### Status: RESOLVIDO

### Sintoma
ROM reconstruída apresentava:
- Artefatos pretos (quadrados) no mapa overworld
- Travamento ao entrar na primeira dungeon

### Causa Raiz
O `binary_rebuilder.py` não processava linhas `.db "string"` do assembly.
Apenas processava `.db $XX` (hex bytes) e instruções.

### Efeito
1,084 bytes eram zero na ROM reconstruída onde deveria ter dados reais.
Esses bytes eram tiles, paletas e dados de mapa → artefatos visuais.

### Correção
Adicionada função `parse_db_string()` e regex `DB_STRING_PATTERN` para
converter caracteres ASCII de strings para bytes.

### Lição Aprendida
> Sempre validar a reconstrução byte-a-byte antes de testar em emulador.
> O relatório de "100% match" estava incorreto porque não contava .db strings.

### Referência
- Arquivo: `tools/validation/binary_rebuilder.py`
- Commits: D001 fix

---

## D002 - Disassembler interpreta dados como BRK

### Data: 2024-07-17
### Severidade: BAIXA
### Status: CONHECIDO (6 bytes afetados)

### Descrição
O disassembler interpreta bytes $00 seguidos de dados como instruções BRK,
perdendo 6 bytes de dados de tile. São 6 bytes em 1MB (0.0006%).

### Impacto
Mínimo - provavelmente não afeta gameplay ou gráficos visíveis.

### Correção Futura
Modificar o disassembler para detectar padrões de dados (tiles 4bpp)
e gerar `.db` em vez de interpretar como código.

### Referência
- Arquivo: `tools/disassembler/full_disasm.py`

---

## Padrões de Validação

### Regra: Sempre validar hash antes de testar

```bash
python3 tools/validation/hash_validator.py <original.sfc> <rebuilt.sfc>
```

### Checklist de Validação
- [ ] MD5 hashes idênticos
- [ ] SNES header válido (checksum + complement = 0xFFFF)
- [ ] 0 bytes diferentes
- [ ] Teste no emulador (início, overworld, dungeon, boss)
