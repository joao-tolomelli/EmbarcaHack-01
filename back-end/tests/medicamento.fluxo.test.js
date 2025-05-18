import request from 'supertest';
import express from 'express';
import authRoutes from '../src/routes/auth.routes.js';
import medicamentoRoutes from '../src/routes/medicamento.routes.js';
import pool from '../src/config/db.js';

const app = express();
app.use(express.json());
app.use('/api/v1/auth', authRoutes);
app.use('/api/v1/medicamentos', medicamentoRoutes);

const testUser = {
  name: 'Farmacêutico de Teste',
  email: 'farmaco@teste.com',
  password: 'senha123',
  profile: 1 // farmacêutico
};

let token = '';
let medicamentoId = null;

beforeAll(async () => {
  await pool.query('DELETE FROM users WHERE email = ?', [testUser.email]);
});

afterAll(async () => {
  await pool.query('DELETE FROM medicamentos WHERE paciente_id IN (SELECT id FROM users WHERE email = ?)', [testUser.email]);
  await pool.query('DELETE FROM users WHERE email = ?', [testUser.email]);
  //await pool.end();
});

describe('Fluxo completo - Medicamentos', () => {
  test('1. Registro do usuário', async () => {
    const res = await request(app)
      .post('/api/v1/auth/register')
      .send(testUser);

    expect(res.statusCode).toBe(201);
  });

  test('2. Login e obtenção do token JWT', async () => {
    const res = await request(app)
      .post('/api/v1/auth/login')
      .send({ email: testUser.email, password: testUser.password });

    expect(res.statusCode).toBe(200);
    expect(res.body.token).toBeDefined();

    token = res.body.token;
  });

  test('3. Criação de medicamento', async () => {
    const res = await request(app)
      .post('/api/v1/medicamentos')
      .set('Authorization', `Bearer ${token}`)
      .send({ name: 'Losartana', color: 'branco', time: '08:00' });

    expect([200, 201]).toContain(res.statusCode);
    medicamentoId = res.body.id;
    expect(medicamentoId).toBeDefined();
  });

  test('4. Listagem de medicamentos', async () => {
    const res = await request(app)
      .get('/api/v1/medicamentos')
      .set('Authorization', `Bearer ${token}`);

    expect(res.statusCode).toBe(200);
    expect(res.body.length).toBeGreaterThan(0);
  });

  test('5. Atualização do medicamento', async () => {
    const res = await request(app)
      .put(`/api/v1/medicamentos/${medicamentoId}`)
      .set('Authorization', `Bearer ${token}`)
      .send({ name: 'Losartana 50mg', color: 'branco', time: '08:30' });

    expect(res.statusCode).toBe(204);
  });

  test('6. Atualização de status do medicamento', async () => {
    const res = await request(app)
      .put(`/api/v1/medicamentos/status/${medicamentoId}`)
      .set('Authorization', `Bearer ${token}`)
      .send({ status: 'ingerido' });

    expect(res.statusCode).toBe(204);
  });

  test('7. Exclusão do medicamento', async () => {
    const res = await request(app)
      .delete(`/api/v1/medicamentos/${medicamentoId}`)
      .set('Authorization', `Bearer ${token}`);

    expect(res.statusCode).toBe(204);
  });
});
