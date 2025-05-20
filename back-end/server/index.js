const express = require('express');
const app = express();
const cron = require('node-cron');
const db = require('./db');
const cors = require("cors");

const mqttClient = require('./mqttClient');
const medicinesRoutes = require('./routes/medicines');
const { syncMedicinesTable } = require('./syncMedicinesTable');

app.use(express.json());
app.use(cors());

app.use('/api/medicines', medicinesRoutes);

// Sincronizar a tabela medicines
syncMedicinesTable();

// Envio a cada 30 minutos dos medicamentos que devem ser ingeridos na próxima hora
cron.schedule('*/30 * * * *', async () => {
  const now = new Date();
  const horaAtual = now.getHours();
  const minutoAtual = now.getMinutes();

  const horaMin = `${horaAtual.toString().padStart(2, '0')}:${minutoAtual.toString().padStart(2, '0')}`;
  const horaMax = `${(horaAtual + 1).toString().padStart(2, '0')}:${minutoAtual.toString().padStart(2, '0')}`;

  const { rows } = await db.query(
    `SELECT * FROM medicines WHERE time >= $1 AND time < $2`,
    [horaMin, horaMax]
  );

  if (rows.length > 0) {
    mqttClient.publish('medicamentos/proximos', JSON.stringify(rows));
    console.log('Enviando medicamentos para a próxima hora:', rows);
  }
});

app.listen(3000, () => {
  console.log('Servidor rodando na porta 3000');
});
