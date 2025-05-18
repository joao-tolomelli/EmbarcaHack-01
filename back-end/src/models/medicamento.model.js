import pool from '../config/db.js';

/**
 * Lista todos os medicamentos cadastrados
 */
export async function listarMedicamentos() {
  const [rows] = await pool.query('SELECT * FROM medicamentos');
  return rows;
}

/**
 * Cadastra um novo medicamento
 */
export async function criarMedicamento({ name, color, time, paciente_id }) {
  const [result] = await pool.query(
    'INSERT INTO medicamentos (name, color, time, status, paciente_id) VALUES (?, ?, ?, ?, ?)',
    [name, color, time, 'pendente', paciente_id]
  );
  return {
    id: result.insertId,
    name,
    color,
    time,
    status: 'pendente',
    paciente_id
  };
}

/**
 * Atualiza um medicamento existente por ID
 */
export async function atualizarMedicamento(id, { name, color, time }) {
  await pool.query(
    'UPDATE medicamentos SET name = ?, color = ?, time = ? WHERE id = ?',
    [name, color, time, id]
  );
}

/**
 * Remove um medicamento por ID
 */
export async function deletarMedicamento(id) {
  await pool.query('DELETE FROM medicamentos WHERE id = ?', [id]);
}

/**
 * Retorna medicamentos agendados para a próxima hora de um paciente
 */
export async function listarProximosMedicamentos(pacienteId) {
  const [rows] = await pool.query(
    `SELECT * FROM medicamentos 
     WHERE paciente_id = ? 
       AND TIME(time) BETWEEN TIME(NOW()) AND TIME(DATE_ADD(NOW(), INTERVAL 1 HOUR))`,
    [pacienteId]
  );
  return rows;
}

/**
 * Atualiza o status de um medicamento
 */
export async function atualizarStatusMedicamento(id, status) {
  await pool.query(
    'UPDATE medicamentos SET status = ? WHERE id = ?',
    [status, id]
  );
}

/**
 * Busca um medicamento específico pelo ID
 */
export async function buscarMedicamentoPorId(id) {
  const [rows] = await pool.query('SELECT * FROM medicamentos WHERE id = ?', [id]);
  return rows[0] || null;
}

export default {
  listarMedicamentos,
  criarMedicamento,
  atualizarMedicamento,
  deletarMedicamento,
  listarProximosMedicamentos,
  atualizarStatusMedicamento,
  buscarMedicamentoPorId
};