function Button({ text }) {
  return (
    <button className="bg-green-main text-white px-4 w-full py-2 rounded-lg transition duration-300 transform hover:bg-green-stroke hover:scale-105 cursor-pointer">
      {text}
    </button>
  );
}

export default Button;
