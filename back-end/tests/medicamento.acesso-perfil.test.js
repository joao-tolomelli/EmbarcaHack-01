import request from 'supertest';
import express from 'express';
import authRoutes from '../src/routes/auth.routes.js';
import medicamentoRoutes from '../src/routes/medicamento.routes.js';
import pool from '../src/config/db.js';

const app = express();
app.use(express.json());
app.use('/api/v1/auth', authRoutes);
app.use('/api/v1/medicamentos', medicamentoRoutes);

const beneficiario = {
  name: 'Paciente Teste',
  email: 'paciente@teste.com',
  password: 'senha123',
  profile: 3
};

let token = '';
let medicamentoId = null;

beforeAll(async () => {
  await pool.query('DELETE FROM users WHERE email = ?', [beneficiario.email]);

  // Registrar beneficiário
  await request(app)
    .post('/api/v1/auth/register')
    .send(beneficiario);

  const res = await request(app)
    .post('/api/v1/auth/login')
    .send({ email: beneficiario.email, password: beneficiario.password });

  token = res.body.token;

  // Força a criação de um medicamento diretamente no banco
  const [rows] = await pool.query(
    `SELECT id FROM users WHERE email = ?`,
    [beneficiario.email]
  );

  const paciente_id = rows[0]?.id;

  const [insert] = await pool.query(
    `INSERT INTO medicamentos (name, color, time, status, paciente_id)
     VALUES (?, ?, ?, ?, ?)`,
    ['Vitamina C', 'laranja', '10:00', 'pendente', paciente_id]
  );

  medicamentoId = insert.insertId;
});

afterAll(async () => {
  await pool.query('DELETE FROM medicamentos WHERE paciente_id IN (SELECT id FROM users WHERE email = ?)', [beneficiario.email]);
  await pool.query('DELETE FROM users WHERE email = ?', [beneficiario.email]);
  //await pool.end();
});

describe('Acesso por perfil - Beneficiário', () => {
  test('Não deve criar medicamento (POST)', async () => {
    const res = await request(app)
      .post('/api/v1/medicamentos')
      .set('Authorization', `Bearer ${token}`)
      .send({ name: 'Proibido', color: 'azul', time: '07:00' });

    expect(res.statusCode).toBe(403);
  });

  test('Não deve editar medicamento (PUT)', async () => {
    const res = await request(app)
      .put(`/api/v1/medicamentos/${medicamentoId}`)
      .set('Authorization', `Bearer ${token}`)
      .send({ name: 'Alterado', color: 'roxo', time: '07:00' });

    expect(res.statusCode).toBe(403);
  });

  test('Não deve excluir medicamento (DELETE)', async () => {
    const res = await request(app)
      .delete(`/api/v1/medicamentos/${medicamentoId}`)
      .set('Authorization', `Bearer ${token}`);

    expect(res.statusCode).toBe(403);
  });

  test('Pode visualizar seus medicamentos (GET)', async () => {
    const res = await request(app)
      .get('/api/v1/medicamentos')
      .set('Authorization', `Bearer ${token}`);

    expect(res.statusCode).toBe(200);
    expect(res.body.length).toBeGreaterThanOrEqual(1);
  });

  test('Pode atualizar status (PUT /status)', async () => {
    const res = await request(app)
      .put(`/api/v1/medicamentos/status/${medicamentoId}`)
      .set('Authorization', `Bearer ${token}`)
      .send({ status: 'ingerido' });

    expect(res.statusCode).toBe(204);
  });
});
