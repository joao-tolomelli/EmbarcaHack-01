const express = require('express');
const router = express.Router();
const db = require('../db');
const mqttClient = require('../mqttClient');

// (GET) listar medicamentos
router.get('/', async (req, res) => {
  const { rows } = await db.query('SELECT * FROM medicines');
  res.json(rows);
});

// (POST) adicionar medicamento
router.post('/', async (req, res) => {
  const { name, color, time, status } = req.body;
  const result = await db.query(
    'INSERT INTO medicines (name, color, time, status) VALUES ($1, $2, $3, $4) RETURNING *',
    [name, color, time, status]
  );
  res.status(201).json(result.rows[0]);
});

// (PUT) atualizar medicamento
router.put('/', async (req, res) => {
  const { id, name, color, time, status } = req.body;
  const result = await db.query(
    'UPDATE medicines SET name=$1, color=$2, time=$3, status=$4 WHERE id=$5 RETURNING *',
    [name, color, time, status, id]
  );
  res.json(result.rows[0]);
});

// (DELETE) deletar medicamento
router.delete('/', async (req, res) => {
  const { id } = req.body;
  await db.query('DELETE FROM medicines WHERE id=$1', [id]);
  res.sendStatus(204);
});

// (GET) enviar medicamentos ignorados via MQTT
router.get('/ignorados', async (req, res) => {
  const { rows } = await db.query('SELECT * FROM medicines WHERE status = $1', ['ignorado']);
  mqttClient.publish('medicamentos/ignorados', JSON.stringify(rows));
  res.json({ enviado: true, medicamentos: rows });
});

module.exports = router;
