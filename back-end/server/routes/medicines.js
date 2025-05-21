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
router.put('/:id', async (req, res) => {
  const { name, color, time, status } = req.body;
  const { id } = req.params;

  try {
    const result = await db.query(
      'UPDATE medicines SET name=$1, color=$2, time=$3, status=$4 WHERE id=$5 RETURNING *',
      [name, color, time, status, id]
    );

    if (result.rows.length === 0) {
      return res.status(404).json({ error: 'Medicamento não encontrado' });
    }

    res.json(result.rows[0]);
  } catch (err) {
    console.error('Erro ao atualizar medicamento:', err);
    res.status(500).json({ error: 'Erro interno no servidor' });
  }
});

// (DELETE) deletar medicamento
router.delete('/:id', async (req, res) => {
  const { id } = req.params;

  try {
    const result = await db.query('DELETE FROM medicines WHERE id = $1 RETURNING *', [id]);

    if (result.rows.length === 0) {
      return res.status(404).json({ error: 'Medicamento não encontrado' });
    }

    res.sendStatus(204); // No Content
  } catch (err) {
    console.error('Erro ao deletar medicamento:', err);
    res.status(500).json({ error: 'Erro interno no servidor' });
  }
});


// (GET) enviar medicamentos ignorados via MQTT
router.get('/ignorados', async (req, res) => {
  const { rows } = await db.query('SELECT * FROM medicines WHERE status = $1', ['ignorado']);
  console.log("enviado")
  mqttClient.publish('medicamentos/ignorados', JSON.stringify(rows));
  res.json({ enviado: true, medicamentos: rows });
});

module.exports = router;
