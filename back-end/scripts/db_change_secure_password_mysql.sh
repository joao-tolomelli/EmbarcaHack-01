#!/bin/bash

# Definir usuário
DB_USER="remedios_db"

# Gerar senha segura e saudável (apenas letras e números, com underscore)
NEW_PASSWORD=$(tr -dc 'A-Za-z0-9_-' < /dev/urandom | head -c 24)

# Exibir a nova senha gerada
echo "Nova senha gerada para o usuário $DB_USER: $NEW_PASSWORD"

# Solicitar senha root do MySQL e aplicar a mudança
mysql -u root -p <<EOF
ALTER USER '${DB_USER}'@'localhost' IDENTIFIED BY '${NEW_PASSWORD}';
FLUSH PRIVILEGES;
EOF

