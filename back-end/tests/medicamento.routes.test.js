// tests/medicamento.routes.test.js

import request from 'supertest';
import jwt from 'jsonwebtoken';
import express from 'express';
import path from 'path';
import { fileURLToPath } from 'url';

import pool from '../src/config/db.js';
import medicamentoRoutes from '../src/routes/medicamento.routes.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Mock do app Express com middleware JSON
const app = express();
app.use(express.json());
app.use('/api/v1/medicamentos', medicamentoRoutes);

const testUser = {
  id: 999,
  name: 'Usuário de Teste',
  email: 'teste@fake.com',
  password: 'senha123',
  profile: 2 // familiar
};

const generateToken = (profile, id = testUser.id) => {
  return jwt.sign(
    { id, email: testUser.email, profile },
    process.env.JWT_SECRET,
    { expiresIn: '1h' }
  );
};

beforeAll(async () => {
  await pool.query(
    `INSERT INTO users (id, name, email, password, profile)
     VALUES (?, ?, ?, ?, ?)
     ON DUPLICATE KEY UPDATE name = VALUES(name), profile = VALUES(profile)`,
    [testUser.id, testUser.name, testUser.email, testUser.password, testUser.profile]
  );
});

afterAll(async () => {
  // Remove medicamentos criados pelo usuário de teste
  await pool.query('DELETE FROM medicamentos WHERE paciente_id = ?', [testUser.id]);

  // Remove o próprio usuário
  await pool.query('DELETE FROM users WHERE id = ?', [testUser.id]);

  // Encerra a conexão do pool
  //await pool.end();
});

describe('Medicamento - Controle de Acesso por Perfil', () => {
  test('Beneficiário (perfil 3) NÃO pode criar medicamento', async () => {
    const token = generateToken(3);
    const response = await request(app)
      .post('/api/v1/medicamentos')
      .set('Authorization', `Bearer ${token}`)
      .send({ name: 'Teste', color: 'azul', time: '08:00' });

    expect(response.statusCode).toBe(403);
  });

  test('Familiar (perfil 2) pode criar medicamento', async () => {
    const token = generateToken(2);
    const response = await request(app)
      .post('/api/v1/medicamentos')
      .set('Authorization', `Bearer ${token}`)
      .send({ name: 'Dipirona', color: 'branco', time: '09:00' });

    expect([200, 201]).toContain(response.statusCode);
  });

  test('Usuário não autenticado deve receber erro 401', async () => {
    const response = await request(app)
      .get('/api/v1/medicamentos');

    expect(response.statusCode).toBe(401);
  });
});

