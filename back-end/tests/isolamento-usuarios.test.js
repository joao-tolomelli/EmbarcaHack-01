import request from 'supertest';
import express from 'express';
import authRoutes from '../src/routes/auth.routes.js';
import medicamentoRoutes from '../src/routes/medicamento.routes.js';
import pool from '../src/config/db.js';

const app = express();
app.use(express.json());
app.use('/api/v1/auth', authRoutes);
app.use('/api/v1/medicamentos', medicamentoRoutes);

const userA = {
  name: 'Usuário A',
  email: 'a@teste.com',
  password: 'senha123',
  profile: 1 // farmacêutico
};

const userB = {
  name: 'Usuário B',
  email: 'b@teste.com',
  password: 'senha123',
  profile: 2 // familiar
};

let tokenA = '';
let tokenB = '';
let medicamentoId = null;

beforeAll(async () => {
  await pool.query('DELETE FROM users WHERE email IN (?, ?)', [userA.email, userB.email]);

  // Registrar e logar A
  await request(app).post('/api/v1/auth/register').send(userA);
  const resA = await request(app).post('/api/v1/auth/login').send({ email: userA.email, password: userA.password });
  tokenA = resA.body.token;

  // Registrar e logar B
  await request(app).post('/api/v1/auth/register').send(userB);
  const resB = await request(app).post('/api/v1/auth/login').send({ email: userB.email, password: userB.password });
  tokenB = resB.body.token;

  // Usuário A cria medicamento
  const resMed = await request(app)
    .post('/api/v1/medicamentos')
    .set('Authorization', `Bearer ${tokenA}`)
    .send({ name: 'Remédio A', color: 'azul', time: '08:00' });

  medicamentoId = resMed.body.id;
});

afterAll(async () => {
  await pool.query('DELETE FROM medicamentos WHERE paciente_id IN (SELECT id FROM users WHERE email IN (?, ?))', [userA.email, userB.email]);
  await pool.query('DELETE FROM users WHERE email IN (?, ?)', [userA.email, userB.email]);
  //await pool.end();
});

describe('Isolamento entre usuários', () => {
  test('Usuário B não pode editar medicamento de A', async () => {
    const res = await request(app)
      .put(`/api/v1/medicamentos/${medicamentoId}`)
      .set('Authorization', `Bearer ${tokenB}`)
      .send({ name: 'Tentativa', color: 'vermelho', time: '09:00' });

    expect(res.statusCode).toBeGreaterThanOrEqual(403);
    expect(res.statusCode).toBeLessThan(405);
  });

  test('Usuário B não pode excluir medicamento de A', async () => {
    const res = await request(app)
      .delete(`/api/v1/medicamentos/${medicamentoId}`)
      .set('Authorization', `Bearer ${tokenB}`);

    expect(res.statusCode).toBeGreaterThanOrEqual(403);
    expect(res.statusCode).toBeLessThan(405);
  });

  test('Usuário B não pode atualizar status do medicamento de A', async () => {
    const res = await request(app)
      .put(`/api/v1/medicamentos/status/${medicamentoId}`)
      .set('Authorization', `Bearer ${tokenB}`)
      .send({ status: 'ingerido' });

    expect(res.statusCode).toBeGreaterThanOrEqual(403);
    expect(res.statusCode).toBeLessThan(405);
  });
});
