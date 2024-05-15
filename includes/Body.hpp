#ifndef BODY_HPP
#define BODY_HPP

class	Body{

	private:
		int			_length;
		int			_expected_length // if the header content length was present will be positive else -1 ?
		bool		_chuncked;

		//choose one_ between both under ?
		std::string	_content;
		int			_fd;

		bool		_is_complete;
		std::string::iterator	_current_it; // if _content is a string and it is being writen somewhere will remember where to start again


	public:

		Body();
		Body(Body const &Copy);
		~Body();
		Body	&operator=(Body const &Rhs);

		int	addContent(std::string buffer); //add the buffer to the body return 1 if it is now complete ?
		int	writeContent(int fd);
		int	writeContent(std::string &output);

};

#endif
