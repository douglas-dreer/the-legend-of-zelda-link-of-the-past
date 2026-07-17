# Erros Comuns e Lições Aprendidas

## Histórico de Erros

### E001 - .db "string" não processado no rebuild
- **Data**: 2024-07-17
- **Causa**: binary_rebuilder.py não convertia ASCII para bytes
- **Impacto**: 1,084 bytes zeros → artefatos pretos no jogo
- **Correção**: Adicionar parse_db_string() com regex DB_STRING_PATTERN
- **Status**: RESOLVIDO - ROM testada e funcionando perfeitamente
- **Validação**: MD5 idêntico, 0 bytes diferentes, 100% match

### E002 - Disassembler perde dados de tile
- **Data**: 2024-07-17
- **Causa**: Bytes $00 interpretados como BRK em vez de dados
- **Impacto**: 6 bytes perdidos (0.0006% da ROM)
- **Status**: Conhecido, correção futura
- **Prevenção**: Verificar se .db cobre todos os bytes não-código

---

## Regras de Ouro

### 1. Validação de ROM
```bash
# SEMPRE executar antes de testar
python3 tools/validation/hash_validator.py original.sfc rebuilt.sfc
```

### 2. Formatos de Dados no Assembly
- `.db $XX` - byte hex (processado corretamente)
- `.db "string" - ASCII (precisa de parse especial)
- `.db $XX, $YY, $ZZ` - múltiplos bytes (processado corretamente)

### 3. Checklists de Build
- [ ] Disassembler gerou assembly completo
- [ ] Binary rebuilder processou todos os formatos .db
- [ ] Hash MD5 é idêntico ao original
- [ ] SNES header checksum válido
- [ ] Teste em emulador: início, overworld, dungeon

### 4. Debug de Artefatos Visuais
1. Comparar ROM original vs reconstruída byte-a-byte
2. Identificar padrão dos diffs (tiles, paletas, etc.)
3. Verificar se o assembly tem .db para esses offsets
4. Corrigir binary rebuilder ou disassembler

---

## Ferramentas de Validação

### hash_validator.py
```bash
python3 tools/validation/hash_validator.py <original> <rebuilt> [output.json]
```
Gera relatório com MD5, SHA1, SHA256, CRC32, e comparação byte-a-byte.

### binary_rebuilder.py
```bash
python3 tools/validation/binary_rebuilder.py
```
Reconstrói ROM a partir do assembly e compara com original.

---

## Referências Rápidas

- **Disassembler**: `tools/disassembler/full_disasm.py`
- **Binary Rebuilder**: `tools/validation/binary_rebuilder.py`
- **Hash Validator**: `tools/validation/hash_validator.py`
- **Discovery Log**: `docs/discovery/README.md`
