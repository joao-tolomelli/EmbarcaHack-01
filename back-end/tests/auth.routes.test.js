import request from 'supertest';
import jwt from 'jsonwebtoken';
import express from 'express';
import authRoutes from '../src/routes/auth.routes.js';
import pool from '../src/config/db.js';

const app = express();
app.use(express.json());
app.use('/api/v1/auth', authRoutes);

const testUser = {
  name: 'Teste Login',
  email: 'login@teste.com',
  password: 'senha123',
  profile: 1
};

let token = '';

beforeAll(async () => {
  // Garante que o usuário não exista antes
  await pool.query('DELETE FROM users WHERE email = ?', [testUser.email]);
});

afterAll(async () => {
  // Remove usuário de teste após todos os testes
  await pool.query('DELETE FROM users WHERE email = ?', [testUser.email]);
//  await pool.end();
});

describe('Autenticação - Registro e Login', () => {
  test('Deve registrar novo usuário com sucesso', async () => {
    const response = await request(app)
      .post('/api/v1/auth/register')
      .send(testUser);

    expect(response.statusCode).toBe(201);
    expect(response.body.message).toMatch(/registrado/i);
  });

  test('Não deve registrar o mesmo e-mail duas vezes', async () => {
    const response = await request(app)
      .post('/api/v1/auth/register')
      .send(testUser);

    expect(response.statusCode).toBe(400);
    expect(response.body.message).toMatch(/já registrado/i);
  });

  test('Deve fazer login com e-mail e senha válidos', async () => {
    const response = await request(app)
      .post('/api/v1/auth/login')
      .send({ email: testUser.email, password: testUser.password });

    expect(response.statusCode).toBe(200);
    expect(response.body.token).toBeDefined();

    token = response.body.token;
  });

  test('Não deve fazer login com senha inválida', async () => {
    const response = await request(app)
      .post('/api/v1/auth/login')
      .send({ email: testUser.email, password: 'senhaErrada' });

    expect(response.statusCode).toBe(401);
    expect(response.body.message).toMatch(/inválidas/i);
  });

  test('Deve retornar dados do usuário logado com token válido', async () => {
    const response = await request(app)
      .get('/api/v1/auth/me')
      .set('Authorization', `Bearer ${token}`);

    expect(response.statusCode).toBe(200);
    expect(response.body.email).toBe(testUser.email);
    expect(response.body.profile).toBe(testUser.profile);
  });

  test('Deve falhar ao acessar /me sem token', async () => {
    const response = await request(app)
      .get('/api/v1/auth/me');

    expect(response.statusCode).toBe(401);
  });
});
