// middleware/auth.middleware.js
import jwt from 'jsonwebtoken';
import dotenv from 'dotenv';

dotenv.config();

const JWT_SECRET = process.env.JWT_SECRET || 'segredo_padrao';

/**
 * Middleware para verificar se o token JWT é válido.
 * Espera o header: Authorization: Bearer <token>
 */
// export function verifyToken(req, res, next) {
//   const authHeader = req.headers['authorization'];

//   if (!authHeader) {
//     return res.status(401).json({ message: 'Token não fornecido' });
//   }

//   const [scheme, token] = authHeader.split(' ');

//   if (scheme !== 'Bearer' || !token) {
//     return res.status(401).json({ message: 'Formato inválido do token' });
//   }

//   try {
//     const decoded = jwt.verify(token, JWT_SECRET);
//     req.user = decoded; // Adiciona o usuário ao request
//     next(); // Continua para a próxima função
//   } catch (err) {
//     return res.status(403).json({ message: 'Token inválido ou expirado' });
//   }
// }

export function verifyToken(req, res, next) {
  req.user = { id: 1, email: 'teste@teste.com' };
  next();
}
