// controllers/auth.controller.js
import jwt from 'jsonwebtoken';
import dotenv from 'dotenv';

dotenv.config();

const JWT_SECRET = process.env.JWT_SECRET || 'segredo_padrao';
const users = []; // Simulação de banco de dados em memória

/**
 * Gera um token JWT válido por 1 hora
 */
function generateToken(user) {
  return jwt.sign(
    { id: user.id, email: user.email },
    JWT_SECRET,
    { expiresIn: '1h' }
  );
}

/**
 * Controlador para login de usuário
 */
export function loginUser(req, res) {
  const { email, password } = req.body;

  const user = users.find(u => u.email === email && u.password === password);
  if (!user) {
    return res.status(401).json({ message: 'Credenciais inválidas' });
  }

  const token = generateToken(user);
  res.json({ token });
}

/**
 * Controlador para registro de novo usuário
 */
export function registerUser(req, res) {
  const { email, password } = req.body;

  const exists = users.some(u => u.email === email);
  if (exists) {
    return res.status(409).json({ message: 'Usuário já registrado' });
  }

  const newUser = {
    id: users.length + 1,
    email,
    password
  };

  users.push(newUser);
  const token = generateToken(newUser);
  res.status(201).json({ token });
}

/**
 * Rota protegida - retorna os dados do usuário logado
 */
export function getUserProfile(req, res) {
  res.json({ user: req.user });
}
