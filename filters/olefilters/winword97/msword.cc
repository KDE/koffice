<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">
<HTML>
<HEAD>
<TITLE>���������� �� KFind: ���������</TITLE>
<META HTTP-EQUIV="content-type" CONTENT="text/html; charset=koi8-r">
<META NAME="keywords" CONTENT="KDE kfind translation Russian Alexander Izyurov ������� ������� ��������� �������">
<META NAME="description" CONTENT="Translation of KDE kfind to Russian by Alexander Izyurov">
</HEAD>
<BODY BGCOLOR="#ffffff" LINK="#aa0000" TEXT="#000000" > 
<FONT FACE="Helvetica">
<A HREF="http://www.kde.org/"><IMG SRC="logotp3.png" BORDER="0" ALT="The K Desktop Environment"></A>
<HR WIDTH="100%" SIZE=2 ALIGN="CENTER" NOSHADE>

 
<P ALIGN="RIGHT">

<A HREF="index-3.html"> ������ </A>
<A HREF="index-1.html"> ����� </A>
<A HREF="index.html#toc2"> ���������� </A>
</P>
<H3><A NAME="s2">2. ���������</A></H3>

<P>
<P>
<H3><A NAME="ss2.1">2.1 ��� ����� KFind</A>
</H3>

<P>KFind -- ����� ������� KDE  
<A HREF="http://www.kde.org">http://www.kde.org</A>. KFind ��������� ��
<A HREF="ftp://ftp.kde.org/pub/kde/">ftp://ftp.kde.org/pub/kde/</A>, �������� ftp-������� ������� KDE.
��� ����� ���� ������ �� ��������� ������.
<P>
<H3><A NAME="ss2.2">2.2 ����������</A>
</H3>

<P>��� ����, ����� ������� �������������� KFind, ��� ����������� ��������� ������
<CODE>libkdecore</CODE> � <CODE>libkfm</CODE>. ��� ����������� ���������� � ���
KFind ����� ����� ��
<A HREF="ftp://ftp.kde.org/pub/kde/">ftp://ftp.kde.org/pub/kde/</A>.
<P>
<H3><A NAME="ss2.3">2.3 ���������� � ���������</A>
</H3>

<P><B>Kfind</B> ������ �������� ��������� ������ KDE � �������
������������� ��������������� ������ � ������� ������������ KDE ��� ����������
��������� KDE.
<P>���� ��� ���������� ������������� � ������������� KFind ������, 
��������� �  ������� ������������ KFind  � ������� ��������� �������:
<BLOCKQUOTE><CODE>
<PRE>
 
% ./configure 
% make 
% make install 
</PRE>
</CODE></BLOCKQUOTE>
<P>��������� KFind ���������� <CODE>autoconf</CODE>, � ��� �� ������ ���������� ������� ���
����������.
���� �� ��� �� ����������� � ����������, ����������, �������� � ��� � ������ �������� <B>KDE</B>.
<P>
<H3><A NAME="ss2.4">2.4 ���������</A>
</H3>

<P>������ ��������� ����������� � ���������� ���� ���������;
����������� � ������� ��� ������������ ����� �����������. 
��� �������������� �������� ����������� �������� ���������
<B>kfind</B> :
<P>
<P><B>Kfind</B> ������ ��������� ������������ �� ����������������� �����
(������  <CODE>~/.kde/share/config/kfindrc</CODE>).
<P>���� ���� �������� ���������� � ���������� ����������� ������,
��������� ����������� � ������ ��������� ������������; 
��� ��������� ����� ������������� �� ����������� ����
<B>���������</B> <CODE>Kfind</CODE>. 
<P>
<P><B>����� ��������:</B> ���� �������� ������� �� ��������.
������ ������ ���������� � ��������� � ���������� �������, ��������, <CODE>[Saving]</CODE>
<P>
<P>���������� ����� ��������� ��������� � 
���������� �������� ������ <CODE>Kfind</CODE>.
<P>
<P><B>��������� ����������:</B> � ���������������� ����� ���������� ��� � ������
�����, ������������� ��� ���������� ����������� ������. ��� ��������� ��������
� ��������� ����:
<BLOCKQUOTE><CODE>
<PRE>
[Saving]
Format=HTML
Filename=/root/result.html
</PRE>
</CODE></BLOCKQUOTE>

��� �������� <CODE>Format</CODE> ����� ��������� �������� <CODE>HTML </CODE>��� <CODE>Plain Text</CODE>.
<P>
<P><B>��������� �����������:</B> ��������� <CODE>kfind</CODE> ���������� �������
� ������� <CODE>[Archiver Types]</CODE> <I>kfindrc</I> ���:
<BLOCKQUOTE><CODE>
<PRE>
[Archiver Types]
Archivers=tar;zip;zoo;
</PRE>
</CODE></BLOCKQUOTE>
<P>������ ��������� �������� ����������� � ����������� �������:
<BLOCKQUOTE><CODE>
<PRE>
[tar]
ExecOnCreate=tar cf %a -C %d %n
ExecOnUpdate=tar uf %a -C %d %n
Comment=Tar
</PRE>
</CODE></BLOCKQUOTE>
<P>������ ������ �������� �������, ������� ����� ���������
��� �������� ������ ������, � ������ -- ��� ��������� ������������� ������.
������ ������ -- �����������, ������� ������������ � �������� �������� ����������
� ���������� ���� <B>���������</B>.
� ��������� �������� ����� ������������ ����������, ������� ���������� 
�� ����� %.
<P>��� ������ ���������� ���������� ��������� ������:
<P>
<P><CODE>%a</CODE> ������ ��� ������
<P>
<P><CODE>%f</CODE> ��� �������� �����
<P>
<P><CODE>%d</CODE> ������������ �������. ���� ���������� �������� ����
(�� �������), �� ����������� ����������� ���������� �����
�������, ���������� ���� ����.
<P>
<P><CODE>%n</CODE> ��� �����. ���� �������� �������� ���������, �� ��� ��� ����� ��������.
<P>
<P>
<P ALIGN="RIGHT">

<A HREF="index-3.html"> ������ </A>
<A HREF="index-1.html"> ����� </A>
<A HREF="index.html#toc2"> ���������� </A>
</P>
<CENTER>
<HR WIDTH="100%" SIZE=3 ALIGN=CENTER NOSHADE>
</CENTER>    
</FONT>

 
</BODY>
</HTML>
