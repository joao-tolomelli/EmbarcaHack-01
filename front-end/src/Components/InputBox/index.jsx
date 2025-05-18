function InputBox({ type = "text", label,value, onChange }) {
  return (
    <div className="flex flex-col">
    <label className="text-sm ml-2" htmlFor={label.toLowerCase()}>
      {label}
    </label>
    <input
      className="border-2 border-black rounded-xl px-3 py-1 focus:outline-none focus:border-blue-600 transition-colors duration-300"
      type={type}
      name={label.toLowerCase()}
      value={value}
      id={label.toLowerCase()}
      onChange={onChange}
    />
  </div>
  );
}

export default InputBox;
